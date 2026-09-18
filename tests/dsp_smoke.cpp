#include "support/TestSupport.h"\n#include "MetalFinisherDSP.h"

#include <cassert>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double sampleRate = 48000.0;

double measureLowCutGain(double frequency) {
    constexpr int warmup = 24000;
    constexpr int measured = 48000;

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(0.0);
    dsp.setLowCut80(true);

    double inputPower = 0.0;
    double outputPower = 0.0;

    for (int i = 0; i < warmup + measured; ++i) {
        const double x =
            std::sin(2.0 * pi * frequency * i / sampleRate);

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
    dsp.setLowCut80(false);

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

    const double lowCut40 =
        measureLowCutGain(40.0);
    const double lowCut1000 =
        measureLowCutGain(1000.0);

    HGGF_REQUIRE(lowCut40 < 0.35);
    HGGF_REQUIRE(lowCut1000 > 0.99);
    HGGF_REQUIRE(lowCut1000 < 1.01);

    std::cout
        << "DSP smoke test passed\n"
        << "Optional 80 Hz low-cut gain at 40 Hz: "
        << lowCut40 << "\n"
        << "Optional 80 Hz low-cut gain at 1 kHz: "
        << lowCut1000 << "\n";

    return 0;
}
