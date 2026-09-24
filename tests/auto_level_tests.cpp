#include "support/TestSupport.h"
#include "AutoLevelCompensator.h"
#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::AutoLevelCompensator;
using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double pi =
    3.141592653589793238462643383279502884;

double linearToDb(double value) {
    return value > 0.0
        ? 20.0 * std::log10(value)
        : -1000.0;
}

void verifyDirectCompensation(
    double sampleRate,
    double processedScale) {

    AutoLevelCompensator level;
    level.prepare(sampleRate);

    long double inputSquares = 0.0L;
    long double outputSquares = 0.0L;
    std::size_t measured = 0;

    const int total =
        static_cast<int>(sampleRate * 8.0);

    const int warmup =
        static_cast<int>(sampleRate * 4.0);

    for (int i = 0; i < total; ++i) {
        const double time =
            static_cast<double>(i) / sampleRate;

        const double reference =
            0.35 * std::sin(2.0 * pi * 110.0 * time) +
            0.25 * std::sin(2.0 * pi * 1050.0 * time) +
            0.15 * std::sin(2.0 * pi * 4800.0 * time);

        double left =
            reference * processedScale;

        double right =
            reference * processedScale;

        level.processFrame(
            reference,
            reference,
            left,
            right);

        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));
        HGGF_REQUIRE(level.currentGainDb() >= -3.0001);
        HGGF_REQUIRE(level.currentGainDb() <= 3.0001);

        if (i >= warmup) {
            inputSquares +=
                static_cast<long double>(reference) * reference;

            outputSquares +=
                static_cast<long double>(left) * left;

            ++measured;
        }
    }

    const double inputRms =
        std::sqrt(
            static_cast<double>(
                inputSquares /
                static_cast<long double>(measured)));

    const double outputRms =
        std::sqrt(
            static_cast<double>(
                outputSquares /
                static_cast<long double>(measured)));

    const double deltaDb =
        linearToDb(outputRms / inputRms);

    if (processedScale >= 0.72 &&
        processedScale <= 1.40) {
        HGGF_REQUIRE(std::abs(deltaDb) < 0.12);
    } else if (processedScale < 0.72) {
        // Heavy artificial loss exceeds the +3 dB safety cap.
        HGGF_REQUIRE(level.currentGainDb() > 2.90);
        HGGF_REQUIRE(deltaDb < -1.0);
    } else {
        // Heavy artificial boost exceeds the -3 dB safety cap.
        HGGF_REQUIRE(level.currentGainDb() < -2.90);
        HGGF_REQUIRE(deltaDb > 1.0);
    }
}

void verifyFastPhraseBootstrap() {
    constexpr double sampleRate = 48000.0;

    for (const double scale :
         {0.75, 1.33}) {

        AutoLevelCompensator level;
        level.prepare(sampleRate);

        const int count =
            static_cast<int>(
                sampleRate * 0.25);

        for (int i = 0; i < count; ++i) {
            const double time =
                static_cast<double>(i) /
                sampleRate;

            const double reference =
                0.35 * std::sin(
                    2.0 * pi * 120.0 * time) +
                0.22 * std::sin(
                    2.0 * pi * 1600.0 * time) +
                0.12 * std::sin(
                    2.0 * pi * 4300.0 * time);

            double left =
                reference * scale;

            double right = left;

            level.processFrame(
                reference,
                reference,
                left,
                right);
        }

        if (scale < 1.0) {
            HGGF_REQUIRE(
                level.currentGainDb() >
                1.5);
        } else {
            HGGF_REQUIRE(
                level.currentGainDb() <
                -1.5);
        }
    }
}

void verifyProgrammeChangeDoesNotPump(
    double sampleRate) {

    AutoLevelCompensator level;
    level.prepare(sampleRate);

    double minGainDb = 1000.0;
    double maxGainDb = -1000.0;
    double maxStepDb = 0.0;
    double previousGainDb = 0.0;

    const int total =
        static_cast<int>(sampleRate * 6.0);

    for (int i = 0; i < total; ++i) {
        const double time =
            static_cast<double>(i) / sampleRate;

        // Alternate every 125 ms between a palm-mute-like denser section and
        // a more open sustain-like section. The processed level deliberately
        // changes in opposite directions so a fast level matcher would pump.
        const int section =
            static_cast<int>(time / 0.125);

        const double processedScale =
            (section & 1) == 0
                ? 0.78
                : 1.22;

        const double envelope =
            (section & 1) == 0
                ? 1.00
                : 0.72;

        const double reference =
            envelope * (
                0.28 * std::sin(2.0 * pi * 115.0 * time) +
                0.22 * std::sin(2.0 * pi * 950.0 * time) +
                0.14 * std::sin(2.0 * pi * 3900.0 * time));

        double left =
            reference * processedScale;
        double right = left;

        level.processFrame(
            reference,
            reference,
            left,
            right);

        const double gainDb =
            level.currentGainDb();

        HGGF_REQUIRE(std::isfinite(gainDb));

        if (i > static_cast<int>(sampleRate * 1.0)) {
            minGainDb =
                std::min(minGainDb, gainDb);

            maxGainDb =
                std::max(maxGainDb, gainDb);

            maxStepDb =
                std::max(
                    maxStepDb,
                    std::abs(
                        gainDb -
                        previousGainDb));
        }

        previousGainDb = gainDb;
    }

    const double excursionDb =
        maxGainDb - minGainDb;

    std::cerr
        << "Programme-change auto-level "
        << sampleRate
        << " Hz excursion / max sample step dB: "
        << excursionDb << " / "
        << maxStepDb << "\n";

    // The matcher must not chase 125 ms phrase changes like a compressor.
    HGGF_REQUIRE(excursionDb < 0.75);
    HGGF_REQUIRE(maxStepDb < 0.01);
}

void verifyFinishZeroAfterMakeup() {
    constexpr double sampleRate = 48000.0;

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);

    for (int i = 0;
         i < static_cast<int>(sampleRate * 3.0);
         ++i) {

        const double time =
            static_cast<double>(i) / sampleRate;

        double left =
            0.6 * std::sin(
                2.0 * pi * 100.0 * time) +
            0.4 * std::sin(
                2.0 * pi * 3400.0 * time);

        double right = left;

        dsp.processFrame(left, right);
    }

    HGGF_REQUIRE(std::isfinite(dsp.currentAutoLevelGainDb()));
    HGGF_REQUIRE(std::abs(dsp.currentAutoLevelGainDb()) <= 3.0001);

    dsp.setFinish(0.0);

    for (int i = 0; i < 4096; ++i) {
        const double input =
            0.5 * std::sin(
                2.0 * pi * 777.0 *
                static_cast<double>(i) /
                sampleRate);

        double left = input;
        double right = -input;

        dsp.processFrame(left, right);

        HGGF_REQUIRE(left == input);
        HGGF_REQUIRE(right == -input);
        HGGF_REQUIRE(dsp.currentAutoLevelGainDb() == 0.0);
    }
}

void verifySilenceReturn() {
    constexpr double sampleRate = 48000.0;

    AutoLevelCompensator level;
    level.prepare(sampleRate);

    for (int i = 0;
         i < static_cast<int>(sampleRate * 4.0);
         ++i) {

        const double time =
            static_cast<double>(i) / sampleRate;

        const double reference =
            0.4 * std::sin(
                2.0 * pi * 440.0 * time);

        double left = reference * 0.90;
        double right = left;

        level.processFrame(
            reference,
            reference,
            left,
            right);
    }

    HGGF_REQUIRE(level.currentGainDb() > 0.5);

    for (int i = 0;
         i < static_cast<int>(sampleRate * 3.0);
         ++i) {

        double left = 0.0;
        double right = 0.0;

        level.processFrame(
            0.0,
            0.0,
            left,
            right);
    }

    HGGF_REQUIRE(level.currentGainDb() < 0.05);
}

} // namespace

int main() {
    for (const double sampleRate :
         {44100.0, 48000.0, 96000.0, 192000.0}) {

        verifyDirectCompensation(
            sampleRate,
            0.90);

        verifyDirectCompensation(
            sampleRate,
            0.50);

        verifyDirectCompensation(
            sampleRate,
            1.10);

        verifyDirectCompensation(
            sampleRate,
            2.00);
    }

    verifyFastPhraseBootstrap();

    for (const double sampleRate :
         {44100.0, 48000.0, 96000.0, 192000.0}) {
        verifyProgrammeChangeDoesNotPump(
            sampleRate);
    }

    verifyFinishZeroAfterMakeup();
    verifySilenceReturn();

    std::cout
        << "Auto-level tests passed\n";

    return 0;
}
