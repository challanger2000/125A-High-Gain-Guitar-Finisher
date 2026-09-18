#include "IndustrialRoom.h"
#include "support/TestSupport.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using HighGainGuitarFinisher::dsp::IndustrialRoom;

namespace {

constexpr double kPi =
    3.141592653589793238462643383279502884;

double db(double value) {
    return 20.0 *
        std::log10(
            std::max(
                value,
                1.0e-20));
}

double rmsWindow(
    const std::vector<double>& left,
    const std::vector<double>& right,
    double sampleRate,
    double startSeconds,
    double endSeconds) {

    const std::size_t begin =
        static_cast<std::size_t>(
            startSeconds *
            sampleRate);

    const std::size_t end =
        std::min<std::size_t>(
            left.size(),
            static_cast<std::size_t>(
                endSeconds *
                sampleRate));

    long double sum = 0.0L;
    std::size_t count = 0;

    for (std::size_t i = begin;
         i < end;
         ++i) {

        sum +=
            0.5L * (
                static_cast<long double>(
                    left[i]) *
                    left[i] +
                static_cast<long double>(
                    right[i]) *
                    right[i]);

        ++count;
    }

    return count > 0
        ? std::sqrt(
            static_cast<double>(
                sum /
                static_cast<long double>(
                    count)))
        : 0.0;
}

double wetToneRms(
    double sampleRate,
    double frequency) {

    IndustrialRoom room;
    room.prepare(sampleRate);
    room.setAmount(1.0);

    const int total =
        static_cast<int>(
            sampleRate *
            3.0);

    const int warmup =
        static_cast<int>(
            sampleRate *
            1.5);

    long double sum = 0.0L;
    std::size_t count = 0;

    for (int i = 0;
         i < total;
         ++i) {

        const double input =
            0.2 *
            std::sin(
                2.0 *
                kPi *
                frequency *
                static_cast<double>(i) /
                sampleRate);

        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            input,
            input,
            wetLeft,
            wetRight);

        HGGF_REQUIRE(
            std::isfinite(wetLeft) &&
            std::isfinite(wetRight));

        if (i >= warmup) {
            sum +=
                0.5L * (
                    static_cast<long double>(
                        wetLeft) *
                        wetLeft +
                    static_cast<long double>(
                        wetRight) *
                        wetRight);

            ++count;
        }
    }

    return std::sqrt(
        static_cast<double>(
            sum /
            static_cast<long double>(
                count)));
}

void verifyRoomZero() {
    IndustrialRoom room;
    room.prepare(48000.0);
    room.setAmount(0.0);

    for (int i = 0;
         i < 48000;
         ++i) {

        const double input =
            0.5 *
            std::sin(
                2.0 *
                kPi *
                777.0 *
                static_cast<double>(i) /
                48000.0);

        double wetLeft = 1.0;
        double wetRight = 1.0;

        room.processFrame(
            input,
            -input,
            wetLeft,
            wetRight);

        HGGF_REQUIRE(
            wetLeft == 0.0);

        HGGF_REQUIRE(
            wetRight == 0.0);
    }
}

void verifyTimingAcrossSampleRates() {
    for (const double sampleRate :
         {44100.0, 48000.0, 96000.0}) {

        IndustrialRoom room;
        room.prepare(sampleRate);
        room.setAmount(1.0);

        const int count =
            static_cast<int>(
                sampleRate *
                0.050);

        int first = -1;

        for (int i = 0;
             i < count;
             ++i) {

            const double input =
                i == 0
                    ? 1.0
                    : 0.0;

            double wetLeft = 0.0;
            double wetRight = 0.0;

            room.processFrame(
                input,
                input,
                wetLeft,
                wetRight);

            if (first < 0 &&
                std::max(
                    std::abs(wetLeft),
                    std::abs(wetRight)) >
                    1.0e-7) {

                first = i;
            }
        }

        HGGF_REQUIRE(first >= 0);

        const double firstMs =
            1000.0 *
            static_cast<double>(first) /
            sampleRate;

        HGGF_REQUIRE(
            firstMs > 8.0 &&
            firstMs < 16.0);
    }
}

void verifyTailClears() {
    constexpr double sampleRate =
        48000.0;

    IndustrialRoom room;
    room.prepare(sampleRate);
    room.setAmount(1.0);

    for (int i = 0;
         i < static_cast<int>(
            sampleRate *
            0.30);
         ++i) {

        const double input =
            i == 0
                ? 1.0
                : 0.0;

        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            input,
            input,
            wetLeft,
            wetRight);
    }

    room.setAmount(0.0);

    double finalPeak = 0.0;

    for (int i = 0;
         i < static_cast<int>(
            sampleRate *
            1.0);
         ++i) {

        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            0.0,
            0.0,
            wetLeft,
            wetRight);

        if (i >
            static_cast<int>(
                sampleRate *
                0.50)) {

            finalPeak =
                std::max(
                    finalPeak,
                    std::max(
                        std::abs(wetLeft),
                        std::abs(wetRight)));
        }
    }

    HGGF_REQUIRE(
        room.currentAmount() <
        1.0e-6);

    HGGF_REQUIRE(
        finalPeak ==
        0.0);
}

} // namespace

int main() {
    verifyRoomZero();
    verifyTimingAcrossSampleRates();
    verifyTailClears();

    constexpr double sampleRate =
        48000.0;

    constexpr double seconds =
        1.5;

    const std::size_t count =
        static_cast<std::size_t>(
            sampleRate *
            seconds);

    IndustrialRoom room;
    room.prepare(sampleRate);
    room.setAmount(1.0);

    std::vector<double>
        left(count, 0.0);

    std::vector<double>
        right(count, 0.0);

    for (std::size_t i = 0;
         i < count;
         ++i) {

        const double input =
            i == 0
                ? 1.0
                : 0.0;

        room.processFrame(
            input,
            input,
            left[i],
            right[i]);

        HGGF_REQUIRE(
            std::isfinite(left[i]) &&
            std::isfinite(right[i]));
    }

    std::size_t first = count;

    for (std::size_t i = 0;
         i < count;
         ++i) {

        if (std::max(
                std::abs(left[i]),
                std::abs(right[i])) >
            1.0e-7) {

            first = i;
            break;
        }
    }

    const double firstMs =
        1000.0 *
        static_cast<double>(first) /
        sampleRate;

    HGGF_REQUIRE(
        firstMs > 8.0 &&
        firstMs < 16.0);

    const double early =
        rmsWindow(
            left,
            right,
            sampleRate,
            0.01,
            0.08);

    const double mid =
        rmsWindow(
            left,
            right,
            sampleRate,
            0.12,
            0.30);

    const double late =
        rmsWindow(
            left,
            right,
            sampleRate,
            0.45,
            0.70);

    const double veryLate =
        rmsWindow(
            left,
            right,
            sampleRate,
            0.90,
            1.20);

    HGGF_REQUIRE(
        early > 1.0e-6);

    HGGF_REQUIRE(
        mid > 1.0e-7);

    HGGF_REQUIRE(
        db(late / mid) <
        -8.0);

    HGGF_REQUIRE(
        db(veryLate / mid) <
        -24.0);

    long double leftEnergy = 0.0L;
    long double rightEnergy = 0.0L;
    long double cross = 0.0L;
    long double monoEnergy = 0.0L;
    long double stereoEnergy = 0.0L;

    for (std::size_t i = 0;
         i < count;
         ++i) {

        leftEnergy +=
            static_cast<long double>(
                left[i]) *
            left[i];

        rightEnergy +=
            static_cast<long double>(
                right[i]) *
            right[i];

        cross +=
            static_cast<long double>(
                left[i]) *
            right[i];

        const long double mono =
            0.5L * (
                left[i] +
                right[i]);

        monoEnergy +=
            mono * mono;

        stereoEnergy +=
            0.5L * (
                static_cast<long double>(
                    left[i]) *
                    left[i] +
                static_cast<long double>(
                    right[i]) *
                    right[i]);
    }

    const double correlation =
        static_cast<double>(
            cross /
            std::sqrt(
                std::max(
                    leftEnergy *
                    rightEnergy,
                    1.0e-30L)));

    const double monoRatioDb =
        10.0 *
        std::log10(
            static_cast<double>(
                monoEnergy /
                std::max(
                    stereoEnergy,
                    1.0e-30L)));

    HGGF_REQUIRE(
        correlation > -0.2 &&
        correlation < 0.90);

    HGGF_REQUIRE(
        monoRatioDb > -4.0);

    const double lowEnergy =
        std::pow(
            wetToneRms(
                sampleRate,
                70.0),
            2.0) +
        std::pow(
            wetToneRms(
                sampleRate,
                110.0),
            2.0) +
        std::pow(
            wetToneRms(
                sampleRate,
                150.0),
            2.0);

    const double midEnergy =
        std::pow(
            wetToneRms(
                sampleRate,
                500.0),
            2.0) +
        std::pow(
            wetToneRms(
                sampleRate,
                1000.0),
            2.0) +
        std::pow(
            wetToneRms(
                sampleRate,
                2000.0),
            2.0);

    const double highEnergy =
        std::pow(
            wetToneRms(
                sampleRate,
                8000.0),
            2.0) +
        std::pow(
            wetToneRms(
                sampleRate,
                10000.0),
            2.0) +
        std::pow(
            wetToneRms(
                sampleRate,
                12000.0),
            2.0);

    const double lowVsMid =
        10.0 *
        std::log10(
            lowEnergy /
            midEnergy);

    const double highVsMid =
        10.0 *
        std::log10(
            highEnergy /
            midEnergy);

    HGGF_REQUIRE(
        lowVsMid < -4.0);

    HGGF_REQUIRE(
        highVsMid < -3.0);

    std::cout
        << "ROOM metrology passed\n"
        << "First reflection: "
        << firstMs
        << " ms\n"
        << "Early/mid/late/very-late RMS dB: "
        << db(early)
        << " / "
        << db(mid)
        << " / "
        << db(late)
        << " / "
        << db(veryLate)
        << "\n"
        << "Wet stereo correlation: "
        << correlation
        << "\n"
        << "Mono/stereo energy: "
        << monoRatioDb
        << " dB\n"
        << "Wet low vs mid: "
        << lowVsMid
        << " dB\n"
        << "Wet high vs mid: "
        << highVsMid
        << " dB\n";

    return 0;
}
