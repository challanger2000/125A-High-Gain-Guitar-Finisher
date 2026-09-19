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

constexpr double kSampleRate = 48000.0;

double db(double value) {
    return 20.0 *
        std::log10(
            std::max(value, 1.0e-20));
}

double rmsWindow(
    const std::vector<double>& left,
    const std::vector<double>& right,
    double startSeconds,
    double endSeconds) {

    const std::size_t begin =
        static_cast<std::size_t>(
            startSeconds * kSampleRate);

    const std::size_t end =
        std::min<std::size_t>(
            left.size(),
            static_cast<std::size_t>(
                endSeconds * kSampleRate));

    long double sum = 0.0L;
    std::size_t count = 0;

    for (std::size_t i = begin; i < end; ++i) {
        sum += 0.5L * (
            static_cast<long double>(left[i]) * left[i] +
            static_cast<long double>(right[i]) * right[i]);
        ++count;
    }

    return count > 0
        ? std::sqrt(static_cast<double>(
            sum / static_cast<long double>(count)))
        : 0.0;
}

double wetToneRms(double frequency) {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setAmount(1.0);

    constexpr int total =
        static_cast<int>(kSampleRate * 3.0);

    constexpr int warmup =
        static_cast<int>(kSampleRate * 1.5);

    long double sum = 0.0L;
    std::size_t count = 0;

    for (int i = 0; i < total; ++i) {
        const double input =
            0.2 * std::sin(
                2.0 * kPi * frequency *
                static_cast<double>(i) /
                kSampleRate);

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
            sum += 0.5L * (
                static_cast<long double>(wetLeft) * wetLeft +
                static_cast<long double>(wetRight) * wetRight);
            ++count;
        }
    }

    return std::sqrt(static_cast<double>(
        sum / static_cast<long double>(count)));
}

void verifyRoomZero() {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setAmount(0.0);

    for (int i = 0; i < 48000; ++i) {
        const double input =
            0.5 * std::sin(
                2.0 * kPi * 777.0 *
                static_cast<double>(i) /
                kSampleRate);

        double wetLeft = 1.0;
        double wetRight = 1.0;

        room.processFrame(
            input,
            -input,
            wetLeft,
            wetRight);

        HGGF_REQUIRE(wetLeft == 0.0);
        HGGF_REQUIRE(wetRight == 0.0);
    }
}

void verifyTimingAcrossSampleRates() {
    for (const double sampleRate :
         {44100.0, 48000.0, 96000.0}) {

        IndustrialRoom room;
        room.prepare(sampleRate);
        room.setAmount(1.0);

        const int count =
            static_cast<int>(sampleRate * 0.10);

        int first = -1;

        for (int i = 0; i < count; ++i) {
            const double input = i == 0 ? 1.0 : 0.0;
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
                    std::abs(wetRight)) > 1.0e-7) {
                first = i;
            }
        }

        HGGF_REQUIRE(first >= 0);

        const double firstMs =
            1000.0 *
            static_cast<double>(first) /
            sampleRate;

        HGGF_REQUIRE(
            firstMs > 12.0 &&
            firstMs < 20.0);
    }
}

void verifyTailClears() {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setAmount(1.0);

    for (int i = 0;
         i < static_cast<int>(kSampleRate * 0.30);
         ++i) {

        const double input = i == 0 ? 1.0 : 0.0;
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
         i < static_cast<int>(kSampleRate * 1.0);
         ++i) {

        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            0.0,
            0.0,
            wetLeft,
            wetRight);

        if (i > static_cast<int>(kSampleRate * 0.60)) {
            finalPeak = std::max(
                finalPeak,
                std::max(
                    std::abs(wetLeft),
                    std::abs(wetRight)));
        }
    }

    HGGF_REQUIRE(room.currentAmount() < 1.0e-6);
    HGGF_REQUIRE(finalPeak == 0.0);
}

double verifyAudibleMaximum() {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setAmount(1.0);

    constexpr int total =
        static_cast<int>(kSampleRate * 4.0);

    constexpr int warmup =
        static_cast<int>(kSampleRate * 1.0);

    long double dryPower = 0.0L;
    long double wetPower = 0.0L;

    for (int i = 0; i < total; ++i) {
        const double time =
            static_cast<double>(i) /
            kSampleRate;

        const double pulse =
            std::fmod(time, 0.25) < 0.080
                ? 1.0
                : 0.30;

        const double left =
            pulse * (
                0.22 * std::sin(2.0 * kPi * 95.0 * time) +
                0.17 * std::sin(2.0 * kPi * 315.0 * time)) +
            0.24 * std::sin(2.0 * kPi * 1050.0 * time) +
            0.16 * std::sin(2.0 * kPi * 3400.0 * time);

        const double right =
            pulse * (
                0.21 * std::sin(2.0 * kPi * 115.0 * time) +
                0.16 * std::sin(2.0 * kPi * 390.0 * time)) +
            0.23 * std::sin(2.0 * kPi * 1080.0 * time) +
            0.15 * std::sin(2.0 * kPi * 4200.0 * time);

        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            left,
            right,
            wetLeft,
            wetRight);

        if (i >= warmup) {
            dryPower += 0.5L * (
                static_cast<long double>(left) * left +
                static_cast<long double>(right) * right);

            wetPower += 0.5L * (
                static_cast<long double>(wetLeft) * wetLeft +
                static_cast<long double>(wetRight) * wetRight);
        }
    }

    const double wetToDryDb =
        10.0 * std::log10(
            static_cast<double>(wetPower / dryPower));

    // ROOM 100 is intentionally an obvious effect setting.
    HGGF_REQUIRE(wetToDryDb > -10.0);
    HGGF_REQUIRE(wetToDryDb < -2.5);

    return wetToDryDb;
}

} // namespace

int main() {
    verifyRoomZero();
    verifyTimingAcrossSampleRates();
    verifyTailClears();

    const double wetToDryDb =
        verifyAudibleMaximum();

    constexpr double seconds = 5.5;

    const std::size_t count =
        static_cast<std::size_t>(
            kSampleRate * seconds);

    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setAmount(1.0);

    std::vector<double> left(count, 0.0);
    std::vector<double> right(count, 0.0);

    for (std::size_t i = 0; i < count; ++i) {
        const double input = i == 0 ? 1.0 : 0.0;

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

    for (std::size_t i = 0; i < count; ++i) {
        if (std::max(
                std::abs(left[i]),
                std::abs(right[i])) > 1.0e-7) {
            first = i;
            break;
        }
    }

    const double firstMs =
        1000.0 *
        static_cast<double>(first) /
        kSampleRate;

    HGGF_REQUIRE(
        firstMs > 12.0 &&
        firstMs < 20.0);

    const double early =
        rmsWindow(left, right, 0.015, 0.12);

    const double mid =
        rmsWindow(left, right, 0.15, 0.40);

    const double late =
        rmsWindow(left, right, 0.50, 0.85);

    const double veryLate =
        rmsWindow(left, right, 1.00, 1.50);

    const double longTail =
        rmsWindow(left, right, 2.00, 2.80);

    const double finalTail =
        rmsWindow(left, right, 4.00, 5.00);

    HGGF_REQUIRE(early > 1.0e-6);
    HGGF_REQUIRE(mid > 1.0e-7);

    const double lateToMid =
        db(late / mid);

    const double veryLateToMid =
        db(veryLate / mid);

    const double longTailToMid =
        db(longTail / mid);

    const double finalToMid =
        db(finalTail / mid);

    // At 100% the room must audibly ring after the guitar stops.
    HGGF_REQUIRE(
        lateToMid > -12.0 &&
        lateToMid < -2.0);

    HGGF_REQUIRE(
        veryLateToMid > -25.0 &&
        veryLateToMid < -7.0);

    HGGF_REQUIRE(
        longTailToMid > -45.0 &&
        longTailToMid < -18.0);

    HGGF_REQUIRE(finalToMid < -35.0);

    long double leftEnergy = 0.0L;
    long double rightEnergy = 0.0L;
    long double cross = 0.0L;
    long double monoEnergy = 0.0L;
    long double stereoEnergy = 0.0L;

    for (std::size_t i = 0; i < count; ++i) {
        leftEnergy +=
            static_cast<long double>(left[i]) * left[i];

        rightEnergy +=
            static_cast<long double>(right[i]) * right[i];

        cross +=
            static_cast<long double>(left[i]) * right[i];

        const long double mono =
            0.5L * (left[i] + right[i]);

        monoEnergy += mono * mono;

        stereoEnergy += 0.5L * (
            static_cast<long double>(left[i]) * left[i] +
            static_cast<long double>(right[i]) * right[i]);
    }

    const double correlation =
        static_cast<double>(
            cross /
            std::sqrt(std::max(
                leftEnergy * rightEnergy,
                1.0e-30L)));

    const double monoRatioDb =
        10.0 * std::log10(
            static_cast<double>(
                monoEnergy /
                std::max(stereoEnergy, 1.0e-30L)));

    HGGF_REQUIRE(
        correlation > -0.2 &&
        correlation < 0.70);

    HGGF_REQUIRE(monoRatioDb > -3.5);

    const double lowEnergy =
        std::pow(wetToneRms(70.0), 2.0) +
        std::pow(wetToneRms(110.0), 2.0) +
        std::pow(wetToneRms(150.0), 2.0);

    const double midEnergy =
        std::pow(wetToneRms(500.0), 2.0) +
        std::pow(wetToneRms(1000.0), 2.0) +
        std::pow(wetToneRms(2000.0), 2.0);

    const double highEnergy =
        std::pow(wetToneRms(8000.0), 2.0) +
        std::pow(wetToneRms(10000.0), 2.0) +
        std::pow(wetToneRms(12000.0), 2.0);

    const double lowVsMid =
        10.0 * std::log10(lowEnergy / midEnergy);

    const double highVsMid =
        10.0 * std::log10(highEnergy / midEnergy);

    HGGF_REQUIRE(lowVsMid < -4.5);
    HGGF_REQUIRE(highVsMid < -4.5);

    std::cout
        << "ROOM metrology passed\n"
        << "Maximum wet/dry: "
        << wetToDryDb
        << " dB\n"
        << "First reflection: "
        << firstMs
        << " ms\n"
        << "Early/mid/late/very-late/long/final RMS dB: "
        << db(early)
        << " / "
        << db(mid)
        << " / "
        << db(late)
        << " / "
        << db(veryLate)
        << " / "
        << db(longTail)
        << " / "
        << db(finalTail)
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
