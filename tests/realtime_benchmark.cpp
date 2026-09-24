#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"
#include "LowCutMapping.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <vector>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;
using HighGainGuitarFinisher::dsp::lowCutNormalizedFromFrequency;

namespace {

constexpr double kPi =
    3.141592653589793238462643383279502884;

using Clock = std::chrono::steady_clock;

double percentile(
    const std::vector<double>& sorted,
    double fraction) {

    HGGF_REQUIRE(!sorted.empty());

    const double clamped =
        std::clamp(fraction, 0.0, 1.0);

    const std::size_t index =
        static_cast<std::size_t>(
            std::llround(
                clamped *
                static_cast<double>(
                    sorted.size() - 1)));

    return sorted[index];
}

double measureTimerOverheadNs() {
    constexpr int iterations = 100000;

    std::vector<double> samples;
    samples.reserve(iterations);

    for (int i = 0; i < iterations; ++i) {
        const auto start = Clock::now();
        const auto end = Clock::now();

        const auto ns =
            std::chrono::duration<double, std::nano>(
                end - start)
                .count();

        HGGF_REQUIRE(std::isfinite(ns));
        HGGF_REQUIRE(ns >= 0.0);
        samples.push_back(ns);
    }

    std::sort(
        samples.begin(),
        samples.end());

    return percentile(samples, 0.50);
}

struct BenchmarkResult {
    double sampleRate {0.0};
    int blockSize {0};
    double deadlineUs {0.0};
    double meanUs {0.0};
    double p95Us {0.0};
    double p99Us {0.0};
    double maxUs {0.0};
    std::size_t overruns {0};
};

BenchmarkResult runBenchmark(
    double sampleRate,
    int blockSize,
    double timerOverheadNs) {

    HGGF_REQUIRE(sampleRate > 1000.0);
    HGGF_REQUIRE(blockSize > 0);

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);
    dsp.setMode(0.5);
    dsp.setMass(1.0);
    dsp.setLowCut(
        lowCutNormalizedFromFrequency(
            90.0));
    dsp.setRoomWet(1.0);
    dsp.setRoomDecay(1.0);

    constexpr int warmupBlocks = 256;
    constexpr int measuredBlocks = 4096;

    double phase = 0.0;
    const double phaseStep =
        2.0 * kPi / sampleRate;

    auto processOneBlock =
        [&](int blockIndex) {

            for (int sample = 0;
                 sample < blockSize;
                 ++sample) {

                const double time =
                    phase;

                const double gate =
                    ((blockIndex / 16) & 1) == 0
                        ? 1.0
                        : 0.35;

                double left =
                    gate * (
                        0.32 *
                            std::sin(
                                107.0 *
                                time) +
                        0.22 *
                            std::sin(
                                337.0 *
                                time)) +
                    0.20 *
                        std::sin(
                            1650.0 *
                            time) +
                    0.19 *
                        std::sin(
                            4200.0 *
                            time) +
                    0.14 *
                        std::sin(
                            8200.0 *
                            time);

                double right =
                    gate * (
                        0.30 *
                            std::sin(
                                119.0 *
                                time) +
                        0.21 *
                            std::sin(
                                371.0 *
                                time)) +
                    0.19 *
                        std::sin(
                            1730.0 *
                            time) +
                    0.18 *
                        std::sin(
                            4470.0 *
                            time) +
                    0.13 *
                        std::sin(
                            8700.0 *
                            time);

                dsp.processFrame(
                    left,
                    right);

                HGGF_REQUIRE(
                    std::isfinite(left));
                HGGF_REQUIRE(
                    std::isfinite(right));

                phase += phaseStep;

                if (phase >
                    2.0 * kPi) {
                    phase -=
                        2.0 * kPi;
                }
            }
        };

    for (int block = 0;
         block < warmupBlocks;
         ++block) {
        processOneBlock(block);
    }

    std::vector<double> blockTimesUs;
    blockTimesUs.reserve(measuredBlocks);

    const double overheadUs =
        timerOverheadNs * 0.001;

    for (int block = 0;
         block < measuredBlocks;
         ++block) {

        const auto start =
            Clock::now();

        processOneBlock(
            block + warmupBlocks);

        const auto end =
            Clock::now();

        double elapsedUs =
            std::chrono::duration<
                double,
                std::micro>(
                    end - start)
                .count();

        elapsedUs =
            std::max(
                0.0,
                elapsedUs -
                    overheadUs);

        HGGF_REQUIRE(
            std::isfinite(elapsedUs));

        blockTimesUs.push_back(
            elapsedUs);
    }

    const double deadlineUs =
        1000000.0 *
        static_cast<double>(blockSize) /
        sampleRate;

    const std::size_t overruns =
        static_cast<std::size_t>(
            std::count_if(
                blockTimesUs.begin(),
                blockTimesUs.end(),
                [deadlineUs](double value) {
                    return value >
                        deadlineUs;
                }));

    const double sum =
        std::accumulate(
            blockTimesUs.begin(),
            blockTimesUs.end(),
            0.0);

    const double mean =
        sum /
        static_cast<double>(
            blockTimesUs.size());

    std::sort(
        blockTimesUs.begin(),
        blockTimesUs.end());

    return {
        sampleRate,
        blockSize,
        deadlineUs,
        mean,
        percentile(
            blockTimesUs,
            0.95),
        percentile(
            blockTimesUs,
            0.99),
        blockTimesUs.back(),
        overruns
    };
}

} // namespace

int main() {
    static_assert(
        Clock::is_steady,
        "Realtime benchmark requires a steady clock");

    const double timerOverheadNs =
        measureTimerOverheadNs();

    HGGF_REQUIRE(
        std::isfinite(timerOverheadNs));
    HGGF_REQUIRE(
        timerOverheadNs >= 0.0);

    std::cout
        << std::fixed
        << std::setprecision(3)
        << "Timer median overhead: "
        << timerOverheadNs
        << " ns\n";

    const double sampleRates[] {
        44100.0,
        48000.0,
        96000.0,
        192000.0
    };

    const int blockSizes[] {
        32,
        64,
        128,
        256
    };

    std::size_t totalOverruns = 0;

    for (const double sampleRate :
         sampleRates) {

        for (const int blockSize :
             blockSizes) {

            const auto result =
                runBenchmark(
                    sampleRate,
                    blockSize,
                    timerOverheadNs);

            totalOverruns +=
                result.overruns;

            std::cout
                << result.sampleRate
                << " Hz / "
                << result.blockSize
                << " samples"
                << " | deadline "
                << result.deadlineUs
                << " us"
                << " | mean "
                << result.meanUs
                << " us"
                << " | p95 "
                << result.p95Us
                << " us"
                << " | p99 "
                << result.p99Us
                << " us"
                << " | max "
                << result.maxUs
                << " us"
                << " | overruns "
                << result.overruns
                << "\n";
        }
    }

    // Shared CI timing is recorded as engineering evidence, not used as a
    // universal shipping threshold. Functional failures still fail this test.
    std::cout
        << "Realtime benchmark completed; total observed CI overruns: "
        << totalOverruns
        << "\n";

    return 0;
}
