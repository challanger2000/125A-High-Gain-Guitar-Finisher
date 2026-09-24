#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"
#include "LowCutMapping.h"

#include <cmath>
#include <iostream>
#include <limits>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;
using HighGainGuitarFinisher::dsp::lowCutNormalizedFromFrequency;

namespace {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double sampleRate = 48000.0;

double measureLowCutGain(
    double cutoffFrequency,
    double probeFrequency,
    double testSampleRate = sampleRate) {

    const int warmup =
        static_cast<int>(
            testSampleRate * 0.5);

    const int measured =
        static_cast<int>(
            testSampleRate * 1.0);

    MetalFinisherDSP dsp;
    dsp.prepare(testSampleRate);
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
                testSampleRate);

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

double measureMassGain(
    double probeFrequency,
    double testSampleRate = sampleRate) {

    const int warmup =
        static_cast<int>(
            testSampleRate * 0.5);

    const int measured =
        static_cast<int>(
            testSampleRate * 1.0);

    MetalFinisherDSP dsp;
    dsp.prepare(testSampleRate);
    dsp.setFinish(0.0);
    dsp.setMass(1.0);
    dsp.setLowCut(0.0);

    double inputPower = 0.0;
    double outputPower = 0.0;

    for (int i = 0; i < warmup + measured; ++i) {
        const double x =
            std::sin(
                2.0 * pi *
                probeFrequency *
                i /
                testSampleRate);

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

    return std::sqrt(
        outputPower /
        inputPower);
}

void verifyLinearMassAmount() {
    MetalFinisherDSP dry;
    MetalFinisherDSP half;
    MetalFinisherDSP full;

    dry.prepare(sampleRate);
    half.prepare(sampleRate);
    full.prepare(sampleRate);

    dry.setFinish(0.0);
    half.setFinish(0.0);
    full.setFinish(0.0);

    dry.setMass(0.0);
    half.setMass(0.5);
    full.setMass(1.0);

    for (int i = 0;
         i < static_cast<int>(
             sampleRate * 2.0);
         ++i) {

        const double time =
            static_cast<double>(i) /
            sampleRate;

        const double inputLeft =
            0.35 * std::sin(
                2.0 * pi * 120.0 * time) +
            0.25 * std::sin(
                2.0 * pi * 250.0 * time) +
            0.20 * std::sin(
                2.0 * pi * 1200.0 * time);

        const double inputRight =
            0.97 * inputLeft +
            0.03 * std::sin(
                2.0 * pi * 310.0 * time);

        double dryLeft = inputLeft;
        double dryRight = inputRight;
        double halfLeft = inputLeft;
        double halfRight = inputRight;
        double fullLeft = inputLeft;
        double fullRight = inputRight;

        dry.processFrame(
            dryLeft,
            dryRight);

        half.processFrame(
            halfLeft,
            halfRight);

        full.processFrame(
            fullLeft,
            fullRight);

        HGGF_REQUIRE(dryLeft == inputLeft);
        HGGF_REQUIRE(dryRight == inputRight);

        const double expectedHalfLeft =
            dryLeft +
            0.5 *
                (fullLeft - dryLeft);

        const double expectedHalfRight =
            dryRight +
            0.5 *
                (fullRight - dryRight);

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

void verifyPathologicalInputSafety() {
    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);
    dsp.setMass(1.0);
    dsp.setLowCut(
        lowCutNormalizedFromFrequency(
            120.0));
    dsp.setRoomWet(1.0);
    dsp.setRoomDecay(1.0);
    dsp.setMode(1.0);

    const double cases[] {
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
        std::numeric_limits<double>::denorm_min(),
        -std::numeric_limits<double>::denorm_min(),
        1.0e300,
        -1.0e300,
        64.0,
        -64.0,
        0.0
    };

    for (int pass = 0; pass < 8; ++pass) {
        for (const double input : cases) {
            double left = input;
            double right = -input;

            dsp.processFrame(
                left,
                right);

            HGGF_REQUIRE(
                std::isfinite(left));
            HGGF_REQUIRE(
                std::isfinite(right));
        }
    }

    // Follow pathological values with normal programme audio. Internal
    // state must recover immediately rather than remain NaN/Inf poisoned.
    for (int i = 0; i < 8192; ++i) {
        const double time =
            static_cast<double>(i) /
            sampleRate;

        double left =
            0.45 * std::sin(
                2.0 * pi * 120.0 * time) +
            0.25 * std::sin(
                2.0 * pi * 4300.0 * time);

        double right =
            0.43 * std::sin(
                2.0 * pi * 145.0 * time) +
            0.23 * std::sin(
                2.0 * pi * 6100.0 * time);

        dsp.processFrame(
            left,
            right);

        HGGF_REQUIRE(
            std::isfinite(left));
        HGGF_REQUIRE(
            std::isfinite(right));
    }
}

void verifyFiniteAcrossSampleRates() {
    for (const double rate :
         {44100.0, 48000.0, 96000.0, 192000.0}) {

        MetalFinisherDSP dsp;
        dsp.prepare(rate);
        dsp.setFinish(1.0);
        dsp.setMass(1.0);

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

double gainDb(double gain) {
    return 20.0 *
        std::log10(
            std::max(
                gain,
                1.0e-20));
}

void verifyStaticToneShapingAcrossSampleRates() {
    const double rates[] {
        44100.0,
        48000.0,
        96000.0,
        192000.0
    };

    const double referenceLowCut =
        measureLowCutGain(
            80.0,
            80.0,
            48000.0);

    const double referenceMass140 =
        measureMassGain(
            140.0,
            48000.0);

    const double referenceMass220 =
        measureMassGain(
            220.0,
            48000.0);

    const double referenceMass1000 =
        measureMassGain(
            1000.0,
            48000.0);

    std::cout
        << "Sample-rate static shaping dB (rate / lowcut80 / mass140 / mass220 / mass1000):\n";

    for (const double rate : rates) {
        const double lowCut =
            measureLowCutGain(
                80.0,
                80.0,
                rate);

        const double mass140 =
            measureMassGain(
                140.0,
                rate);

        const double mass220 =
            measureMassGain(
                220.0,
                rate);

        const double mass1000 =
            measureMassGain(
                1000.0,
                rate);

        const double lowCutDelta =
            gainDb(lowCut) -
            gainDb(referenceLowCut);

        const double mass140Delta =
            gainDb(mass140) -
            gainDb(referenceMass140);

        const double mass220Delta =
            gainDb(mass220) -
            gainDb(referenceMass220);

        const double mass1000Delta =
            gainDb(mass1000) -
            gainDb(referenceMass1000);

        HGGF_REQUIRE(
            std::abs(lowCutDelta) <
            0.02);

        HGGF_REQUIRE(
            std::abs(mass140Delta) <
            0.08);

        HGGF_REQUIRE(
            std::abs(mass220Delta) <
            0.08);

        HGGF_REQUIRE(
            std::abs(mass1000Delta) <
            0.08);

        std::cout
            << "  "
            << rate
            << " / "
            << gainDb(lowCut)
            << " / "
            << gainDb(mass140)
            << " / "
            << gainDb(mass220)
            << " / "
            << gainDb(mass1000)
            << "\n";
    }
}

} // namespace

int main() {
    verifyExactTransparency();
    verifyLinearFinishAmount();
    verifyLinearMassAmount();
    verifyFinishReenableStartsClean();
    verifyPathologicalInputSafety();
    verifyFiniteAcrossSampleRates();
    verifyStaticToneShapingAcrossSampleRates();

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

    const double massAt140 =
        measureMassGain(140.0);

    const double massAt220 =
        measureMassGain(220.0);

    const double massAt280 =
        measureMassGain(280.0);

    const double massAt1000 =
        measureMassGain(1000.0);

    HGGF_REQUIRE(
        massAt140 > 1.55 &&
        massAt140 < 1.75);

    HGGF_REQUIRE(
        massAt220 > 0.43 &&
        massAt220 < 0.55);

    HGGF_REQUIRE(
        massAt280 > 0.44 &&
        massAt280 < 0.56);

    HGGF_REQUIRE(
        massAt1000 > 0.84 &&
        massAt1000 < 0.93);

    std::cout
        << "DSP smoke test passed\n"
        << "FINISH 0/50/100 linear amount law passed for all modes\n"
        << "MASS 0/50/100 linear amount law passed\n"
        << "NaN/Inf/extreme/denormal safety passed\n"
        << "MASS gain at 140/220/280/1000 Hz: "
        << massAt140 << " / "
        << massAt220 << " / "
        << massAt280 << " / "
        << massAt1000 << "\n"
        << "Low Cut gain at cutoff 45/80/120 Hz: "
        << cutoff45 << " / "
        << cutoff80 << " / "
        << cutoff120 << "\n"
        << "80 Hz compatibility response at 40 Hz / 1 kHz: "
        << lowCut80At40 << " / "
        << lowCut80At1000 << "\n";

    return 0;
}
