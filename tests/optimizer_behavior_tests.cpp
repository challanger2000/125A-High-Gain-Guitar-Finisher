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
    double finalArticulationCorrection {0.0};
    double finalArticulationDominance {0.0};
    double maxHarsh {0.0};
    double maxFizz {0.0};
};

Result runScenario(
    double low,
    double body,
    double articulation,
    double harsh,
    double fizz,
    bool pulseLow,
    double mode = 0.0) {

    MetalFinisherDSP dsp;
    dsp.prepare(kSampleRate);
    dsp.setFinish(1.0);
    dsp.setMode(mode);

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

        result.finalArticulationCorrection =
            dsp.currentArticulationCorrection();

        result.finalArticulationDominance =
            dsp.currentArticulationDominance();

        result.maxHarsh = std::max(
            result.maxHarsh,
            dsp.currentHarshnessReduction());

        result.maxFizz = std::max(
            result.maxFizz,
            dsp.currentFizzReduction());
    }

    return result;
}

struct ModeSpectrum {
    double body {0.0};
    double articulation {0.0};
    double harsh {0.0};
    double fizz {0.0};
};

struct EdgeProtectionResult {
    double subRatio {1.0};
    double airRatio {1.0};
};

EdgeProtectionResult measureEdgeProtection() {
    MetalFinisherDSP dsp;
    dsp.prepare(kSampleRate);
    dsp.setFinish(1.0);
    dsp.setMode(0.0);

    constexpr double subHz = 45.0;
    constexpr double airHz = 14000.0;

    constexpr int warmup =
        static_cast<int>(kSampleRate * 3.0);

    constexpr int measured =
        static_cast<int>(kSampleRate * 3.0);

    double inSubSin = 0.0;
    double inSubCos = 0.0;
    double outSubSin = 0.0;
    double outSubCos = 0.0;

    double inAirSin = 0.0;
    double inAirCos = 0.0;
    double outAirSin = 0.0;
    double outAirCos = 0.0;

    for (int i = 0;
         i < warmup + measured;
         ++i) {

        const double time =
            static_cast<double>(i) /
            kSampleRate;

        const double input =
            0.12 * std::sin(
                2.0 * kPi * subHz * time) +
            0.48 * std::sin(
                2.0 * kPi * 350.0 * time) +
            0.04 * std::sin(
                2.0 * kPi * 1650.0 * time) +
            0.48 * std::sin(
                2.0 * kPi * 3900.0 * time) +
            0.40 * std::sin(
                2.0 * kPi * 7800.0 * time) +
            0.08 * std::sin(
                2.0 * kPi * airHz * time);

        double left = input;
        double right = input;

        dsp.processFrame(left, right);

        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));

        if (i >= warmup) {
            const double subPhase =
                2.0 * kPi * subHz * time;

            const double airPhase =
                2.0 * kPi * airHz * time;

            const double subSin =
                std::sin(subPhase);

            const double subCos =
                std::cos(subPhase);

            const double airSin =
                std::sin(airPhase);

            const double airCos =
                std::cos(airPhase);

            inSubSin += input * subSin;
            inSubCos += input * subCos;
            outSubSin += left * subSin;
            outSubCos += left * subCos;

            inAirSin += input * airSin;
            inAirCos += input * airCos;
            outAirSin += left * airSin;
            outAirCos += left * airCos;
        }
    }

    const auto magnitude =
        [measured](double sinSum,
                   double cosSum) {
            return
                2.0 /
                static_cast<double>(measured) *
                std::sqrt(
                    sinSum * sinSum +
                    cosSum * cosSum);
        };

    const double inSub =
        magnitude(
            inSubSin,
            inSubCos);

    const double outSub =
        magnitude(
            outSubSin,
            outSubCos);

    const double inAir =
        magnitude(
            inAirSin,
            inAirCos);

    const double outAir =
        magnitude(
            outAirSin,
            outAirCos);

    return {
        outSub / inSub,
        outAir / inAir
    };
}

ModeSpectrum measureModeSpectrum(double mode) {
    MetalFinisherDSP dsp;
    dsp.prepare(kSampleRate);
    dsp.setFinish(1.0);
    dsp.setMode(mode);

    constexpr double bodyHz = 350.0;
    constexpr double articulationHz = 1650.0;
    constexpr double harshHz = 3900.0;
    constexpr double fizzHz = 7800.0;

    constexpr int warmup =
        static_cast<int>(kSampleRate * 2.0);

    constexpr int measured =
        static_cast<int>(kSampleRate * 3.0);

    std::array<double, 4> sinSum {};
    std::array<double, 4> cosSum {};

    const std::array<double, 4> frequencies {
        bodyHz,
        articulationHz,
        harshHz,
        fizzHz
    };

    for (int i = 0;
         i < warmup + measured;
         ++i) {

        const double time =
            static_cast<double>(i) /
            kSampleRate;

        const double input =
            0.45 * std::sin(
                2.0 * kPi * bodyHz * time) +
            0.03 * std::sin(
                2.0 * kPi * articulationHz * time) +
            0.50 * std::sin(
                2.0 * kPi * harshHz * time) +
            0.42 * std::sin(
                2.0 * kPi * fizzHz * time);

        double left = input;
        double right = input;

        dsp.processFrame(left, right);

        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));

        if (i >= warmup) {
            for (std::size_t band = 0;
                 band < frequencies.size();
                 ++band) {

                const double phase =
                    2.0 * kPi *
                    frequencies[band] *
                    time;

                sinSum[band] +=
                    left * std::sin(phase);

                cosSum[band] +=
                    left * std::cos(phase);
            }
        }
    }

    std::array<double, 4> magnitude {};

    for (std::size_t band = 0;
         band < magnitude.size();
         ++band) {

        magnitude[band] =
            2.0 /
            static_cast<double>(measured) *
            std::sqrt(
                sinSum[band] * sinSum[band] +
                cosSum[band] * cosSum[band]);
    }

    return {
        magnitude[0],
        magnitude[1],
        magnitude[2],
        magnitude[3]
    };
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

    std::cerr
        << "Articulation-thin measured dominance: "
        << articulationThin.finalArticulationDominance
        << ", correction: "
        << articulationThin.minArticulationSupport
        << "\n";

    const auto articulationBalanced =
        runScenario(
            0.12,
            0.22,
            0.18,
            0.16,
            0.08,
            false);

    const auto articulationStrong =
        runScenario(
            0.12,
            0.22,
            0.40,
            0.16,
            0.08,
            false);

    std::cerr
        << "Articulation dominance thin/balanced/strong: "
        << articulationThin.finalArticulationDominance << " / "
        << articulationBalanced.finalArticulationDominance << " / "
        << articulationStrong.finalArticulationDominance << "\n"
        << "Articulation correction final thin/balanced/strong: "
        << articulationThin.finalArticulationCorrection << " / "
        << articulationBalanced.finalArticulationCorrection << " / "
        << articulationStrong.finalArticulationCorrection << "\n";

    HGGF_REQUIRE(
        articulationThin.finalArticulationDominance <
        articulationBalanced.finalArticulationDominance);

    HGGF_REQUIRE(
        articulationBalanced.finalArticulationDominance <
        articulationStrong.finalArticulationDominance);

    // Thin articulation must receive an audible but bounded support boost.
    HGGF_REQUIRE(
        articulationThin.finalArticulationCorrection <
        -0.10);

    // A balanced fixture must not be pushed upward by the support path.
    HGGF_REQUIRE(
        articulationBalanced.finalArticulationCorrection >
        -0.03);

    // A broadly balanced fixture is not a tonal reference. It is a safety
    // regression: no single adaptive zone should jump close to its maximum
    // authority when there is no deliberately exaggerated problem.
    const auto alreadyBalanced =
        runScenario(
            0.16,
            0.22,
            0.18,
            0.16,
            0.10,
            false);

    std::cerr
        << "Already-balanced intervention low/body/articulation/harsh/fizz: "
        << alreadyBalanced.maxLow << " / "
        << alreadyBalanced.maxBodyCut << " / "
        << alreadyBalanced.finalArticulationCorrection << " / "
        << alreadyBalanced.maxHarsh << " / "
        << alreadyBalanced.maxFizz << "\n";

    HGGF_REQUIRE(alreadyBalanced.maxLow < 0.20);
    HGGF_REQUIRE(alreadyBalanced.maxBodyCut < 0.22);
    HGGF_REQUIRE(
        std::abs(
            alreadyBalanced.finalArticulationCorrection) <
        0.12);
    HGGF_REQUIRE(alreadyBalanced.maxHarsh < 0.25);
    HGGF_REQUIRE(alreadyBalanced.maxFizz < 0.25);

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
        multiProblem.finalArticulationCorrection <
        -0.05);
    HGGF_REQUIRE(multiProblem.maxHarsh > 0.04);
    HGGF_REQUIRE(multiProblem.maxFizz > 0.04);

    const auto edgeProtection =
        measureEdgeProtection();

    const double subDeltaDb =
        20.0 * std::log10(
            edgeProtection.subRatio);

    const double airDeltaDb =
        20.0 * std::log10(
            edgeProtection.airRatio);

    std::cerr
        << "Protected sub/air delta dB: "
        << subDeltaDb << " / "
        << airDeltaDb << "\n";

    HGGF_REQUIRE(
        std::abs(subDeltaDb) < 0.35);

    HGGF_REQUIRE(
        std::abs(airDeltaDb) < 0.35);

    const auto mode1 =
        measureModeSpectrum(0.0);

    const auto mode2 =
        measureModeSpectrum(0.5);

    const auto mode3 =
        measureModeSpectrum(1.0);

    std::cerr
        << "Mode spectra body/articulation/harsh/fizz\n"
        << "  Mode 1: "
        << mode1.body << " / "
        << mode1.articulation << " / "
        << mode1.harsh << " / "
        << mode1.fizz << "\n"
        << "  Mode 2: "
        << mode2.body << " / "
        << mode2.articulation << " / "
        << mode2.harsh << " / "
        << mode2.fizz << "\n"
        << "  Mode 3: "
        << mode3.body << " / "
        << mode3.articulation << " / "
        << mode3.harsh << " / "
        << mode3.fizz << "\n";

    const double mode1HarshToBody =
        mode1.harsh / mode1.body;

    const double mode2HarshToBody =
        mode2.harsh / mode2.body;

    const double mode1FizzToHarsh =
        mode1.fizz / mode1.harsh;

    const double mode2FizzToHarsh =
        mode2.fizz / mode2.harsh;

    // Mode 2 is the bite profile: clearly more upper-mid attack relative to
    // body, but not a simple broadband brightness/fizz boost.
    HGGF_REQUIRE(
        mode2HarshToBody >
        mode1HarshToBody * 1.05);

    HGGF_REQUIRE(
        mode2FizzToHarsh <
        mode1FizzToHarsh * 0.98);

    // Mode 3 is smooth/controlled: it keeps more body than Mode 1 while
    // remaining clearly less forward, but it must not collapse into an
    // over-dark profile.
    HGGF_REQUIRE(
        mode3.body >
        mode1.body * 1.10);

    HGGF_REQUIRE(
        (mode3.articulation / mode3.body) <
        (mode1.articulation / mode1.body) * 0.90);

    HGGF_REQUIRE(
        (mode3.harsh / mode3.body) <
        (mode1.harsh / mode1.body) * 0.85);

    HGGF_REQUIRE(
        (mode3.fizz / mode3.body) <
        (mode1.fizz / mode1.body) * 0.85);

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
        << "Already-balanced low/body/articulation/harsh/fizz: "
        << alreadyBalanced.maxLow << " / "
        << alreadyBalanced.maxBodyCut << " / "
        << alreadyBalanced.finalArticulationCorrection << " / "
        << alreadyBalanced.maxHarsh << " / "
        << alreadyBalanced.maxFizz << "\n"
        << "Multi-problem low/body/articulation/harsh/fizz: "
        << multiProblem.maxLow << " / "
        << multiProblem.maxBodyCut << " / "
        << multiProblem.finalArticulationCorrection << " / "
        << multiProblem.maxHarsh << " / "
        << multiProblem.maxFizz << "\n"
        << "Protected sub/air delta dB: "
        << subDeltaDb << " / "
        << airDeltaDb << "\n"
        << "Mode 1 body/articulation/harsh/fizz: "
        << mode1.body << " / "
        << mode1.articulation << " / "
        << mode1.harsh << " / "
        << mode1.fizz << "\n"
        << "Mode 2 body/articulation/harsh/fizz: "
        << mode2.body << " / "
        << mode2.articulation << " / "
        << mode2.harsh << " / "
        << mode2.fizz << "\n"
        << "Mode 3 body/articulation/harsh/fizz: "
        << mode3.body << " / "
        << mode3.articulation << " / "
        << mode3.harsh << " / "
        << mode3.fizz << "\n";

    return 0;
}
