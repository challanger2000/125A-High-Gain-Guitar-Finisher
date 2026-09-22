#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double kPi =
    3.141592653589793238462643383279502884;
constexpr double kSampleRate = 48000.0;

struct Result {
    double maxLow {0.0};
    double maxBodyCut {0.0};
    double minBodySupport {0.0};
    double minArticulationSupport {0.0};
    double maxHarsh {0.0};
    double maxFizz {0.0};
};

Result runScenario(
    double low,
    double body,
    double articulation,
    double harsh,
    double fizz,
    bool pulseLow) {

    MetalFinisherDSP dsp;
    dsp.prepare(kSampleRate);
    dsp.setFinish(1.0);

    Result result;

    const int total =
        static_cast<int>(kSampleRate * 5.0);

    for (int i = 0; i < total; ++i) {
        const double t =
            static_cast<double>(i) / kSampleRate;

        const double pulse =
            pulseLow &&
            std::fmod(t, 0.25) < 0.065
                ? 1.0
                : (pulseLow ? 0.18 : 1.0);

        const double x =
            pulse * low *
                std::sin(2.0 * kPi * 125.0 * t) +
            body *
                std::sin(2.0 * kPi * 350.0 * t) +
            articulation *
                std::sin(2.0 * kPi * 1650.0 * t) +
            harsh *
                std::sin(2.0 * kPi * 3900.0 * t) +
            fizz *
                std::sin(2.0 * kPi * 7800.0 * t);

        double left = x;
        double right = x;

        dsp.processFrame(left, right);

        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));

        result.maxLow = std::max(
            result.maxLow,
            dsp.currentDynamicLowEndReduction());

        result.maxBodyCut = std::max(
            result.maxBodyCut,
            dsp.currentBodyReduction());

        result.minBodySupport = std::min(
            result.minBodySupport,
            dsp.currentBodyReduction());

        result.minArticulationSupport = std::min(
            result.minArticulationSupport,
            dsp.currentArticulationCorrection());

        result.maxHarsh = std::max(
            result.maxHarsh,
            dsp.currentHarshnessReduction());

        result.maxFizz = std::max(
            result.maxFizz,
            dsp.currentFizzReduction());
    }

    return result;
}

} // namespace

int main() {
    const auto chugHeavy =
        runScenario(
            0.65,
            0.18,
            0.18,
            0.12,
            0.06,
            true);

    HGGF_REQUIRE(chugHeavy.maxLow > 0.12);

    const auto bodyHeavy =
        runScenario(
            0.10,
            0.60,
            0.18,
            0.10,
            0.05,
            false);

    HGGF_REQUIRE(bodyHeavy.maxBodyCut > 0.08);

    const auto articulationThin =
        runScenario(
            0.12,
            0.22,
            0.015,
            0.16,
            0.08,
            false);

    HGGF_REQUIRE(
        articulationThin.minArticulationSupport <
        -0.02);

    const auto harshFizzHeavy =
        runScenario(
            0.10,
            0.18,
            0.18,
            0.52,
            0.45,
            false);

    HGGF_REQUIRE(harshFizzHeavy.maxHarsh > 0.05);
    HGGF_REQUIRE(harshFizzHeavy.maxFizz > 0.05);

    const auto multiProblem =
        runScenario(
            0.70,
            0.45,
            0.03,
            0.50,
            0.42,
            true);

    // Multiple independent issues must remain active at the same time.
    HGGF_REQUIRE(multiProblem.maxLow > 0.10);
    HGGF_REQUIRE(multiProblem.maxBodyCut > 0.05);
    HGGF_REQUIRE(
        multiProblem.minArticulationSupport <
        -0.01);
    HGGF_REQUIRE(multiProblem.maxHarsh > 0.04);
    HGGF_REQUIRE(multiProblem.maxFizz > 0.04);

    std::cout
        << "Optimizer behavior tests passed\n"
        << "Chug reduction: "
        << chugHeavy.maxLow << "\n"
        << "Body reduction: "
        << bodyHeavy.maxBodyCut << "\n"
        << "Articulation support: "
        << articulationThin.minArticulationSupport << "\n"
        << "Harsh reduction: "
        << harshFizzHeavy.maxHarsh << "\n"
        << "Fizz reduction: "
        << harshFizzHeavy.maxFizz << "\n"
        << "Multi-problem low/body/articulation/harsh/fizz: "
        << multiProblem.maxLow << " / "
        << multiProblem.maxBodyCut << " / "
        << multiProblem.minArticulationSupport << " / "
        << multiProblem.maxHarsh << " / "
        << multiProblem.maxFizz << "\n";

    return 0;
}
