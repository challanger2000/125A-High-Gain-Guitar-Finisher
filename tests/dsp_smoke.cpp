#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"
#include "LowCutMapping.h"

#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;
using HighGainGuitarFinisher::dsp::lowCutNormalizedFromFrequency;

namespace {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double sampleRate = 48000.0;

double measureLowCutGain(
    double cutoffFrequency,
    double probeFrequency) {

    constexpr int warmup = 24000;
    constexpr int measured = 48000;

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(0.0);
    dsp.setLowCut(
        lowCutNormalizedFromFrequency(
            cutoffFrequency));

    double inputPower = 0.0;
    double outputPower = 0.0;

    for (int i = 0; i < warmup + measured; ++i) {
        const double x =
            std::sin(
                2.0 * pi *
                probeFrequency *
                i /
                sampleRate);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);

        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));

        if (i >= warmup) {
            inputPower += x * x;
            outputPower += left * left;
        }
    }

    return std::sqrt(outputPower / inputPower);
}

void verifyExactTransparency() {
    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(0.0);
    dsp.setLowCut(0.0);

    for (int i = 0; i < 4000; ++i) {
        const double x =
            std::sin(2.0 * pi * 440.0 * i / sampleRate);

        double left = x;
        double right = -x;

        dsp.processFrame(left, right);

        HGGF_REQUIRE(left == x);
        HGGF_REQUIRE(right == -x);
    }
}

void verifyFiniteAcrossSampleRates() {
    for (const double rate :
         {44100.0, 48000.0, 96000.0, 192000.0}) {

        MetalFinisherDSP dsp;
        dsp.prepare(rate);
        dsp.setFinish(1.0);

        for (int i = 0;
             i < static_cast<int>(rate * 0.25);
             ++i) {

            const double time =
                static_cast<double>(i) / rate;

            double left =
                0.4 * std::sin(2.0 * pi * 110.0 * time) +
                0.3 * std::sin(2.0 * pi * 4100.0 * time);

            double right =
                0.4 * std::sin(2.0 * pi * 145.0 * time) +
                0.3 * std::sin(2.0 * pi * 6200.0 * time);

            dsp.processFrame(left, right);

            HGGF_REQUIRE(std::isfinite(left));
            HGGF_REQUIRE(std::isfinite(right));
        }
    }
}

} // namespace

int main() {
    verifyExactTransparency();
    verifyFiniteAcrossSampleRates();

    const double cutoff45 =
        measureLowCutGain(45.0, 45.0);

    const double cutoff80 =
        measureLowCutGain(80.0, 80.0);

    const double cutoff120 =
        measureLowCutGain(120.0, 120.0);

    const double lowCut80At40 =
        measureLowCutGain(80.0, 40.0);

    const double lowCut80At1000 =
        measureLowCutGain(80.0, 1000.0);

    HGGF_REQUIRE(cutoff45 > 0.68 && cutoff45 < 0.73);
    HGGF_REQUIRE(cutoff80 > 0.68 && cutoff80 < 0.73);
    HGGF_REQUIRE(cutoff120 > 0.68 && cutoff120 < 0.73);

    HGGF_REQUIRE(lowCut80At40 < 0.35);
    HGGF_REQUIRE(lowCut80At1000 > 0.99);
    HGGF_REQUIRE(lowCut80At1000 < 1.01);

    std::cout
        << "DSP smoke test passed\n"
        << "Low Cut gain at cutoff 45/80/120 Hz: "
        << cutoff45 << " / "
        << cutoff80 << " / "
        << cutoff120 << "\n"
        << "80 Hz compatibility response at 40 Hz / 1 kHz: "
        << lowCut80At40 << " / "
        << lowCut80At1000 << "\n";

    return 0;
}
