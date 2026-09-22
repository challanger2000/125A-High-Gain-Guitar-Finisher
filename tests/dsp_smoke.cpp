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

void verifyLinearFinishAmount() {
    for (const double mode :
         {0.0, 0.5, 1.0}) {

        MetalFinisherDSP dry;
        MetalFinisherDSP half;
        MetalFinisherDSP full;

        dry.prepare(sampleRate);
        half.prepare(sampleRate);
        full.prepare(sampleRate);

        dry.setMode(mode);
        half.setMode(mode);
        full.setMode(mode);

        dry.setFinish(0.0);
        half.setFinish(0.5);
        full.setFinish(1.0);

        for (int i = 0;
             i < static_cast<int>(sampleRate * 3.0);
             ++i) {

            const double time =
                static_cast<double>(i) /
                sampleRate;

            const double pulse =
                std::fmod(time, 0.25) < 0.065
                    ? 1.0
                    : 0.18;

            const double inputLeft =
                pulse * 0.55 *
                    std::sin(2.0 * pi * 125.0 * time) +
                0.38 *
                    std::sin(2.0 * pi * 350.0 * time) +
                0.06 *
                    std::sin(2.0 * pi * 1650.0 * time) +
                0.42 *
                    std::sin(2.0 * pi * 3900.0 * time) +
                0.32 *
                    std::sin(2.0 * pi * 7800.0 * time);

            const double inputRight =
                0.97 * inputLeft +
                0.02 *
                    std::sin(2.0 * pi * 5400.0 * time);

            double dryLeft = inputLeft;
            double dryRight = inputRight;
            double halfLeft = inputLeft;
            double halfRight = inputRight;
            double fullLeft = inputLeft;
            double fullRight = inputRight;

            dry.processFrame(dryLeft, dryRight);
            half.processFrame(halfLeft, halfRight);
            full.processFrame(fullLeft, fullRight);

            HGGF_REQUIRE(dryLeft == inputLeft);
            HGGF_REQUIRE(dryRight == inputRight);

            const double expectedHalfLeft =
                dryLeft +
                0.5 * (fullLeft - dryLeft);

            const double expectedHalfRight =
                dryRight +
                0.5 * (fullRight - dryRight);

            HGGF_REQUIRE(
                std::abs(
                    halfLeft -
                    expectedHalfLeft) <
                1.0e-12);

            HGGF_REQUIRE(
                std::abs(
                    halfRight -
                    expectedHalfRight) <
                1.0e-12);
        }
    }
}

void verifyFinishReenableStartsClean() {
    MetalFinisherDSP reused;
    reused.prepare(sampleRate);
    reused.setFinish(1.0);

    for (int i = 0; i < 24000; ++i) {
        const double t =
            static_cast<double>(i) / sampleRate;

        double left =
            0.7 * std::sin(2.0 * pi * 175.0 * t) +
            0.4 * std::sin(2.0 * pi * 6600.0 * t);

        double right =
            0.6 * std::sin(2.0 * pi * 145.0 * t) +
            0.35 * std::sin(2.0 * pi * 5400.0 * t);

        reused.processFrame(left, right);
    }

    reused.setFinish(0.0);
    reused.setFinish(1.0);

    MetalFinisherDSP fresh;
    fresh.prepare(sampleRate);
    fresh.setFinish(1.0);

    for (int i = 0; i < 4096; ++i) {
        const double t =
            static_cast<double>(i) / sampleRate;

        const double inputLeft =
            0.4 * std::sin(2.0 * pi * 110.0 * t) +
            0.2 * std::sin(2.0 * pi * 3200.0 * t);

        const double inputRight =
            0.38 * std::sin(2.0 * pi * 180.0 * t) +
            0.22 * std::sin(2.0 * pi * 6800.0 * t);

        double reusedLeft = inputLeft;
        double reusedRight = inputRight;
        double freshLeft = inputLeft;
        double freshRight = inputRight;

        reused.processFrame(
            reusedLeft,
            reusedRight);

        fresh.processFrame(
            freshLeft,
            freshRight);

        HGGF_REQUIRE(reusedLeft == freshLeft);
        HGGF_REQUIRE(reusedRight == freshRight);
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
    verifyLinearFinishAmount();
    verifyFinishReenableStartsClean();
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
        << "FINISH 0/50/100 linear amount law passed for all modes\n"
        << "Low Cut gain at cutoff 45/80/120 Hz: "
        << cutoff45 << " / "
        << cutoff80 << " / "
        << cutoff120 << "\n"
        << "80 Hz compatibility response at 40 Hz / 1 kHz: "
        << lowCut80At40 << " / "
        << lowCut80At1000 << "\n";

    return 0;
}
