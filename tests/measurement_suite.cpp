#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"
#include "support/AudioMeasurements.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double sampleRate = 48000.0;
constexpr int seconds = 6;
constexpr std::size_t sampleCount =
    static_cast<std::size_t>(sampleRate * seconds);

void makeReference(
    std::vector<double>& left,
    std::vector<double>& right) {

    left.resize(sampleCount);
    right.resize(sampleCount);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        const double time =
            static_cast<double>(i) / sampleRate;

        const double low =
            0.18 * std::sin(
                2.0 * HGGFTests::kPi * 95.0 * time);

        const double lowMid =
            0.16 * std::sin(
                2.0 * HGGFTests::kPi * 315.0 * time);

        const double mid =
            0.22 * std::sin(
                2.0 * HGGFTests::kPi * 1050.0 * time);

        const double harsh =
            0.14 * std::sin(
                2.0 * HGGFTests::kPi * 4800.0 * time);

        const double presence =
            0.08 * std::sin(
                2.0 * HGGFTests::kPi * 7200.0 * time);

        const double pulse =
            std::fmod(time, 0.25) < 0.070
                ? 1.0
                : 0.30;

        left[i] =
            pulse * (low + lowMid) +
            mid +
            harsh +
            presence;

        right[i] =
            pulse * (
                0.96 * low +
                1.03 * lowMid) +
            0.98 * mid +
            1.04 * harsh +
            0.95 * presence;
    }
}

void process(
    std::vector<double>& left,
    std::vector<double>& right,
    double finish,
    double& finalMakeupDb) {

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(finish);
    dsp.setLowCut(0.0);

    for (std::size_t i = 0; i < left.size(); ++i)
        dsp.processFrame(left[i], right[i]);

    finalMakeupDb =
        dsp.currentAutoLevelGainDb();
}

std::vector<double> monoFromStereo(
    const std::vector<double>& left,
    const std::vector<double>& right) {

    const std::size_t count =
        std::min(left.size(), right.size());

    std::vector<double> mono(count);

    for (std::size_t i = 0; i < count; ++i)
        mono[i] = 0.5 * (left[i] + right[i]);

    return mono;
}

} // namespace

int main() {
    std::vector<double> inputLeft;
    std::vector<double> inputRight;
    makeReference(inputLeft, inputRight);

    auto bypassLeft = inputLeft;
    auto bypassRight = inputRight;

    double bypassMakeupDb = 0.0;

    process(
        bypassLeft,
        bypassRight,
        0.0,
        bypassMakeupDb);

    HGGF_REQUIRE(
        HGGFTests::nullPeak(inputLeft, bypassLeft) ==
        0.0);

    HGGF_REQUIRE(
        HGGFTests::nullPeak(inputRight, bypassRight) ==
        0.0);

    HGGF_REQUIRE(bypassMakeupDb == 0.0);

    auto outputLeft = inputLeft;
    auto outputRight = inputRight;

    double makeupDb = 0.0;

    process(
        outputLeft,
        outputRight,
        1.0,
        makeupDb);

    const auto inputMetrics =
        HGGFTests::measureStereo(
            inputLeft,
            inputRight);

    const auto outputMetrics =
        HGGFTests::measureStereo(
            outputLeft,
            outputRight);

    const auto inputBands =
        HGGFTests::measureTonalBands(
            monoFromStereo(inputLeft, inputRight),
            sampleRate);

    const auto outputBands =
        HGGFTests::measureTonalBands(
            monoFromStereo(outputLeft, outputRight),
            sampleRate);

    const double rmsDelta =
        HGGFTests::linearToDb(
            outputMetrics.rms / inputMetrics.rms);

    const double subDelta =
        HGGFTests::energyDeltaDb(
            inputBands.sub20To80,
            outputBands.sub20To80);

    const double lowDelta =
        HGGFTests::energyDeltaDb(
            inputBands.low80To250,
            outputBands.low80To250);

    const double bodyDelta =
        HGGFTests::energyDeltaDb(
            inputBands.body250To500,
            outputBands.body250To500);

    const double midsDelta =
        HGGFTests::energyDeltaDb(
            inputBands.mids500To2000,
            outputBands.mids500To2000);

    const double upperMidsDelta =
        HGGFTests::energyDeltaDb(
            inputBands.upperMids2000To5000,
            outputBands.upperMids2000To5000);

    const double presenceDelta =
        HGGFTests::energyDeltaDb(
            inputBands.presence5000To8000,
            outputBands.presence5000To8000);

    HGGF_REQUIRE(std::isfinite(rmsDelta));
    HGGF_REQUIRE(std::isfinite(outputMetrics.correlation));
    HGGF_REQUIRE(std::isfinite(outputMetrics.crestDb));

    // Auto-level must remove loudness bias without forcing exact peak matching.
    HGGF_REQUIRE(rmsDelta > -0.35);
    HGGF_REQUIRE(rmsDelta < 0.35);
    HGGF_REQUIRE(makeupDb >= -3.0001);
    HGGF_REQUIRE(makeupDb <= 3.0001);

    // Safety bounds: the optimizer may now make deliberate, audible tonal
    // changes, but one stage must not destroy an entire broad frequency area.
    HGGF_REQUIRE(std::abs(subDelta) < 3.0);
    HGGF_REQUIRE(std::abs(bodyDelta) < 5.0);
    HGGF_REQUIRE(std::abs(midsDelta) < 5.0);
    HGGF_REQUIRE(std::abs(upperMidsDelta) < 5.0);
    HGGF_REQUIRE(std::abs(presenceDelta) < 5.0);

    // This fixture contains intentional palm-mute excess; the low band must
    // therefore be measurably reduced even after level matching.
    HGGF_REQUIRE(lowDelta < -0.20);

    HGGF_REQUIRE(
        std::abs(
            outputMetrics.correlation -
            inputMetrics.correlation) < 0.05);

    std::cout
        << "Measurement suite passed\n"
        << "RMS delta: " << rmsDelta << " dB\n"
        << "Auto-level makeup: "
        << makeupDb << " dB\n"
        << "Crest input/output: "
        << inputMetrics.crestDb << " / "
        << outputMetrics.crestDb << " dB\n"
        << "Correlation input/output: "
        << inputMetrics.correlation << " / "
        << outputMetrics.correlation << "\n"
        << "Band deltas dB:\n"
        << "  20-80: " << subDelta << "\n"
        << "  80-250: " << lowDelta << "\n"
        << "  250-500: " << bodyDelta << "\n"
        << "  500-2000: " << midsDelta << "\n"
        << "  2000-5000: " << upperMidsDelta << "\n"
        << "  5000-8000: " << presenceDelta << "\n";

    return 0;
}
