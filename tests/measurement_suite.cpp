#include "MetalFinisherDSP.h"
#include "support/AudioMeasurements.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double sampleRate = 48000.0;
constexpr int seconds = 4;
constexpr std::size_t sampleCount =
    static_cast<std::size_t>(sampleRate * seconds);

void makeReference(std::vector<double>& left,
                   std::vector<double>& right) {
    left.resize(sampleCount);
    right.resize(sampleCount);

    for (std::size_t i = 0; i < sampleCount; ++i) {
        const double t = static_cast<double>(i) / sampleRate;

        // Deterministic high-gain-like stress signal:
        // low thump + body + mid definition + harshness + upper presence.
        const double low =
            0.18 * std::sin(2.0 * HGGFTests::kPi * 95.0 * t);
        const double lowMid =
            0.16 * std::sin(2.0 * HGGFTests::kPi * 315.0 * t);
        const double mid =
            0.22 * std::sin(2.0 * HGGFTests::kPi * 1050.0 * t);
        const double harsh =
            0.14 * std::sin(2.0 * HGGFTests::kPi * 4800.0 * t);
        const double presence =
            0.08 * std::sin(2.0 * HGGFTests::kPi * 7200.0 * t);

        const double pulse =
            (std::fmod(t, 0.25) < 0.070) ? 1.0 : 0.30;

        left[i] =
            pulse * (low + lowMid) + mid + harsh + presence;

        right[i] =
            pulse * (
                0.96 * low +
                1.03 * lowMid) +
            0.98 * mid +
            1.04 * harsh +
            0.95 * presence;
    }
}

void process(std::vector<double>& left,
             std::vector<double>& right,
             double finish) {
    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(finish);

    for (std::size_t i = 0; i < left.size(); ++i)
        dsp.processFrame(left[i], right[i]);
}

std::vector<double> monoFromStereo(const std::vector<double>& left,
                                   const std::vector<double>& right) {
    const std::size_t count = std::min(left.size(), right.size());
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
    process(bypassLeft, bypassRight, 0.0);

    assert(HGGFTests::nullPeak(inputLeft, bypassLeft) == 0.0);
    assert(HGGFTests::nullPeak(inputRight, bypassRight) == 0.0);

    auto outputLeft = inputLeft;
    auto outputRight = inputRight;
    process(outputLeft, outputRight, 1.0);

    const auto inputMetrics =
        HGGFTests::measureStereo(inputLeft, inputRight);
    const auto outputMetrics =
        HGGFTests::measureStereo(outputLeft, outputRight);

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

    assert(std::isfinite(rmsDelta));
    assert(std::isfinite(outputMetrics.correlation));
    assert(std::isfinite(outputMetrics.crestDb));

    // Guardrails, not tonal targets: catch accidental broadband destruction.
    assert(rmsDelta > -8.0);
    assert(rmsDelta < 2.0);
    assert(midsDelta > -2.0);
    assert(presenceDelta > -3.0);

    // Current FINISH intent: clean low/low-mid and tame harshness more than mids.
    assert(subDelta < -1.0);
    assert(lowDelta < -1.0);
    assert(bodyDelta < -1.0);
    assert(upperMidsDelta < -0.2);

    // Stereo-linked processing must not radically change stereo behaviour.
    assert(std::abs(
        outputMetrics.correlation -
        inputMetrics.correlation) < 0.05);

    std::cout
        << "Measurement suite passed\n"
        << "RMS delta: " << rmsDelta << " dB\n"
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
