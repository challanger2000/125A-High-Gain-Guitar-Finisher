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

struct DensityMetrics {
    double medianDb {0.0};
    double p10Db {0.0};
    double p90Db {0.0};
    double spreadDb {0.0};
    double emptyFraction {0.0};
};

DensityMetrics roomDensity(
    const std::vector<double>& left,
    const std::vector<double>& right,
    double startSeconds,
    double endSeconds) {

    constexpr double binSeconds = 0.005;
    const std::size_t binSamples =
        static_cast<std::size_t>(
            kSampleRate * binSeconds);

    const std::size_t begin =
        static_cast<std::size_t>(
            startSeconds * kSampleRate);

    const std::size_t end =
        std::min<std::size_t>(
            left.size(),
            static_cast<std::size_t>(
                endSeconds * kSampleRate));

    std::vector<double> bins;

    for (std::size_t pos = begin;
         pos + binSamples <= end;
         pos += binSamples) {

        long double sum = 0.0L;

        for (std::size_t i = pos;
             i < pos + binSamples;
             ++i) {

            sum += 0.5L * (
                static_cast<long double>(left[i]) * left[i] +
                static_cast<long double>(right[i]) * right[i]);
        }

        const double rms =
            std::sqrt(
                static_cast<double>(
                    sum /
                    static_cast<long double>(
                        binSamples)));

        bins.push_back(
            db(rms));
    }

    HGGF_REQUIRE(
        bins.size() >= 20);

    auto sorted = bins;

    std::sort(
        sorted.begin(),
        sorted.end());

    const auto percentile =
        [&](double fraction) {
            const std::size_t index =
                static_cast<std::size_t>(
                    std::llround(
                        fraction *
                        static_cast<double>(
                            sorted.size() - 1)));

            return sorted[
                std::min(
                    index,
                    sorted.size() - 1)];
        };

    const double p10 =
        percentile(0.10);

    const double median =
        percentile(0.50);

    const double p90 =
        percentile(0.90);

    const double silenceFloor =
        median - 30.0;

    const std::size_t emptyBins =
        static_cast<std::size_t>(
            std::count_if(
                bins.begin(),
                bins.end(),
                [silenceFloor](double value) {
                    return value <
                        silenceFloor;
                }));

    return {
        median,
        p10,
        p90,
        p90 - p10,
        static_cast<double>(
            emptyBins) /
            static_cast<double>(
                bins.size())
    };
}

double wetToneRms(double frequency) {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setWetDry(1.0);
    room.setDecay(1.0);

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
    room.setWetDry(0.0);
    room.setDecay(1.0);

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


void verifyAdaptiveDucking() {
    IndustrialRoom quiet;
    quiet.prepare(kSampleRate);
    quiet.setWetDry(1.0);
    quiet.setDecay(0.6);

    for (int i = 0;
         i < static_cast<int>(kSampleRate * 2.0);
         ++i) {

        const double t =
            static_cast<double>(i) / kSampleRate;

        const double input =
            0.02 * std::sin(
                2.0 * kPi * 900.0 * t);

        double wetLeft = 0.0;
        double wetRight = 0.0;

        quiet.processFrame(
            input,
            input,
            wetLeft,
            wetRight);
    }

    HGGF_REQUIRE(
        quiet.currentDuckGain() > 0.95);

    IndustrialRoom loud;
    loud.prepare(kSampleRate);
    loud.setWetDry(1.0);
    loud.setDecay(0.6);

    for (int i = 0;
         i < static_cast<int>(kSampleRate * 2.0);
         ++i) {

        const double t =
            static_cast<double>(i) / kSampleRate;

        const double input =
            0.55 * std::sin(
                2.0 * kPi * 900.0 * t);

        double wetLeft = 0.0;
        double wetRight = 0.0;

        loud.processFrame(
            input,
            input,
            wetLeft,
            wetRight);
    }

    HGGF_REQUIRE(
        loud.currentDuckGain() < 0.75);
    HGGF_REQUIRE(
        loud.currentDuckGain() >= 0.67);
}

void verifyGuitarProgrammeStress() {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setWetDry(1.0);
    room.setDecay(0.70);

    const int activeSamples =
        static_cast<int>(kSampleRate * 2.0);

    double minimumDuck = 1.0;
    double activeWetPeak = 0.0;

    for (int i = 0; i < activeSamples; ++i) {
        const double t =
            static_cast<double>(i) / kSampleRate;

        const double local =
            std::fmod(t, 0.25);

        const bool palmMute =
            local < 0.070;

        const double envelope =
            palmMute ? 1.0 : 0.42;

        const double left =
            envelope * (
                0.30 * std::sin(2.0 * kPi * 105.0 * t) +
                0.20 * std::sin(2.0 * kPi * 330.0 * t)) +
            0.20 * std::sin(2.0 * kPi * 1200.0 * t) +
            0.13 * std::sin(2.0 * kPi * 3900.0 * t);

        const double right =
            envelope * (
                0.28 * std::sin(2.0 * kPi * 118.0 * t) +
                0.19 * std::sin(2.0 * kPi * 370.0 * t)) +
            0.19 * std::sin(2.0 * kPi * 1280.0 * t) +
            0.12 * std::sin(2.0 * kPi * 4300.0 * t);

        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            left,
            right,
            wetLeft,
            wetRight);

        HGGF_REQUIRE(std::isfinite(wetLeft));
        HGGF_REQUIRE(std::isfinite(wetRight));

        minimumDuck =
            std::min(
                minimumDuck,
                room.currentDuckGain());

        activeWetPeak =
            std::max(
                activeWetPeak,
                std::max(
                    std::abs(wetLeft),
                    std::abs(wetRight)));
    }

    HGGF_REQUIRE(minimumDuck < 0.80);
    HGGF_REQUIRE(activeWetPeak > 1.0e-4);

    double tailPeak = 0.0;
    double recoveredDuck = room.currentDuckGain();

    const int tailSamples =
        static_cast<int>(kSampleRate * 1.5);

    for (int i = 0; i < tailSamples; ++i) {
        double wetLeft = 0.0;
        double wetRight = 0.0;

        room.processFrame(
            0.0,
            0.0,
            wetLeft,
            wetRight);

        HGGF_REQUIRE(std::isfinite(wetLeft));
        HGGF_REQUIRE(std::isfinite(wetRight));

        if (i < static_cast<int>(kSampleRate * 0.40)) {
            tailPeak =
                std::max(
                    tailPeak,
                    std::max(
                        std::abs(wetLeft),
                        std::abs(wetRight)));
        }

        if (i ==
            static_cast<int>(kSampleRate * 0.30)) {
            recoveredDuck =
                room.currentDuckGain();
        }
    }

    // A hard stop must reveal a real room tail while the ducking detector
    // releases back toward unity instead of holding the room suppressed.
    HGGF_REQUIRE(tailPeak > 1.0e-5);
    HGGF_REQUIRE(recoveredDuck > 0.90);
    HGGF_REQUIRE(
        recoveredDuck >
        minimumDuck + 0.10);

    std::cerr
        << "ROOM guitar stress min/recovered duck, tail peak: "
        << minimumDuck << " / "
        << recoveredDuck << " / "
        << tailPeak << "\n";
}

void verifyTimingAcrossSampleRates() {
    for (const double sampleRate :
         {44100.0, 48000.0, 96000.0, 192000.0}) {

        IndustrialRoom room;
        room.prepare(sampleRate);
        room.setWetDry(1.0);
    room.setDecay(1.0);

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

struct RoomRateSignature {
    double firstMs {0.0};
    double lateToMidDb {0.0};
    double longToMidDb {0.0};
};

RoomRateSignature measureRateSignature(
    double sampleRate) {

    IndustrialRoom room;
    room.prepare(sampleRate);
    room.setWetDry(1.0);
    room.setDecay(1.0);

    const std::size_t count =
        static_cast<std::size_t>(
            sampleRate * 3.2);

    std::vector<double> left(
        count,
        0.0);

    std::vector<double> right(
        count,
        0.0);

    std::size_t first = count;

    for (std::size_t i = 0;
         i < count;
         ++i) {

        const double input =
            i == 0 ? 1.0 : 0.0;

        room.processFrame(
            input,
            input,
            left[i],
            right[i]);

        if (first == count &&
            std::max(
                std::abs(left[i]),
                std::abs(right[i])) >
                1.0e-7) {

            first = i;
        }
    }

    HGGF_REQUIRE(
        first < count);

    const auto rmsAt =
        [&](double startSeconds,
            double endSeconds) {

            const std::size_t begin =
                static_cast<std::size_t>(
                    startSeconds *
                    sampleRate);

            const std::size_t end =
                std::min<std::size_t>(
                    count,
                    static_cast<std::size_t>(
                        endSeconds *
                        sampleRate));

            long double sum = 0.0L;
            std::size_t samples = 0;

            for (std::size_t i = begin;
                 i < end;
                 ++i) {

                sum += 0.5L * (
                    static_cast<long double>(
                        left[i]) *
                        left[i] +
                    static_cast<long double>(
                        right[i]) *
                        right[i]);

                ++samples;
            }

            return samples > 0
                ? std::sqrt(
                    static_cast<double>(
                        sum /
                        static_cast<long double>(
                            samples)))
                : 0.0;
        };

    const double mid =
        rmsAt(
            0.15,
            0.40);

    const double late =
        rmsAt(
            0.50,
            0.85);

    const double longTail =
        rmsAt(
            2.00,
            2.80);

    HGGF_REQUIRE(mid > 0.0);
    HGGF_REQUIRE(late > 0.0);
    HGGF_REQUIRE(longTail > 0.0);

    return {
        1000.0 *
            static_cast<double>(
                first) /
            sampleRate,
        db(late / mid),
        db(longTail / mid)
    };
}

void measureRoomAcrossSampleRates() {
    std::cerr
        << "ROOM sample-rate signature rate/first-ms/late-mid/long-mid dB:\n";

    for (const double sampleRate :
         {44100.0,
          48000.0,
          96000.0,
          192000.0}) {

        const auto signature =
            measureRateSignature(
                sampleRate);

        HGGF_REQUIRE(
            std::isfinite(
                signature.firstMs));

        HGGF_REQUIRE(
            std::isfinite(
                signature.lateToMidDb));

        HGGF_REQUIRE(
            std::isfinite(
                signature.longToMidDb));

        std::cerr
            << "  "
            << sampleRate
            << " / "
            << signature.firstMs
            << " / "
            << signature.lateToMidDb
            << " / "
            << signature.longToMidDb
            << "\n";
    }
}

void verifyTailClears() {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setWetDry(1.0);
    room.setDecay(1.0);

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

    room.setWetDry(0.0);
    room.setDecay(1.0);

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

    HGGF_REQUIRE(room.currentWetDry() < 1.0e-6);
    HGGF_REQUIRE(finalPeak == 0.0);
}

double verifyAudibleMaximum() {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setWetDry(1.0);
    room.setDecay(1.0);

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
    HGGF_REQUIRE(wetToDryDb > -12.0);
    HGGF_REQUIRE(wetToDryDb < -2.0);

    return wetToDryDb;
}

double longTailForDecay(double decay) {
    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setWetDry(1.0);
    room.setDecay(decay);

    constexpr double seconds = 3.2;

    const std::size_t count =
        static_cast<std::size_t>(
            kSampleRate * seconds);

    std::vector<double> left(count, 0.0);
    std::vector<double> right(count, 0.0);

    for (std::size_t i = 0; i < count; ++i) {
        const double input = i == 0 ? 1.0 : 0.0;

        room.processFrame(
            input,
            input,
            left[i],
            right[i]);
    }

    return rmsWindow(
        left,
        right,
        2.0,
        2.8);
}

void verifyIndependentDecay() {
    const double shortTail =
        longTailForDecay(0.20);

    const double longTail =
        longTailForDecay(1.0);

    HGGF_REQUIRE(longTail > 1.0e-10);
    HGGF_REQUIRE(
        longTail >
        shortTail * 4.0);
}

} // namespace

int main() {
    verifyRoomZero();
    verifyTimingAcrossSampleRates();
    measureRoomAcrossSampleRates();
    verifyTailClears();
    verifyAdaptiveDucking();
    verifyGuitarProgrammeStress();
    verifyIndependentDecay();

    const double wetToDryDb =
        verifyAudibleMaximum();

    constexpr double seconds = 6.5;

    const std::size_t count =
        static_cast<std::size_t>(
            kSampleRate * seconds);

    IndustrialRoom room;
    room.prepare(kSampleRate);
    room.setWetDry(1.0);
    room.setDecay(1.0);

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

    const double hostTailEnd =
        rmsWindow(left, right, 5.50, 6.00);

    const double postHostTail =
        rmsWindow(left, right, 6.00, 6.40);

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
        lateToMid < 2.0);

    HGGF_REQUIRE(
        veryLateToMid > -28.0 &&
        veryLateToMid < -4.0);

    HGGF_REQUIRE(
        longTailToMid > -40.0 &&
        longTailToMid < -10.0);

    HGGF_REQUIRE(finalToMid < -28.0);

    const double hostTailEndDb =
        db(hostTailEnd);

    const double postHostTailDb =
        db(postHostTail);

    HGGF_REQUIRE(
        std::isfinite(hostTailEndDb));
    HGGF_REQUIRE(
        std::isfinite(postHostTailDb));

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

    const auto densityEarlyLate =
        roomDensity(
            left,
            right,
            0.12,
            0.40);

    const auto densityLate =
        roomDensity(
            left,
            right,
            0.40,
            1.00);

    HGGF_REQUIRE(
        densityEarlyLate.emptyFraction == 0.0);
    HGGF_REQUIRE(
        densityLate.emptyFraction == 0.0);

    HGGF_REQUIRE(
        densityEarlyLate.spreadDb < 9.0);
    HGGF_REQUIRE(
        densityLate.spreadDb < 11.0);

    std::cerr
        << "ROOM density 120-400 ms median/p10/p90/spread/empty: "
        << densityEarlyLate.medianDb << " / "
        << densityEarlyLate.p10Db << " / "
        << densityEarlyLate.p90Db << " / "
        << densityEarlyLate.spreadDb << " / "
        << densityEarlyLate.emptyFraction << "\n"
        << "ROOM density 400-1000 ms median/p10/p90/spread/empty: "
        << densityLate.medianDb << " / "
        << densityLate.p10Db << " / "
        << densityLate.p90Db << " / "
        << densityLate.spreadDb << " / "
        << densityLate.emptyFraction << "\n";

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
        << "Host-tail 5.5-6.0 / post-tail 6.0-6.4 RMS dB: "
        << hostTailEndDb
        << " / "
        << postHostTailDb
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
