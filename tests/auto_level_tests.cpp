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

    HGGF_REQUIRE(dsp.currentAutoLevelGainDb() > 0.0);

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
         {44100.0, 48000.0, 96000.0}) {

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

    verifyFinishZeroAfterMakeup();
    verifySilenceReturn();

    std::cout
        << "Auto-level tests passed\n";

    return 0;
}
