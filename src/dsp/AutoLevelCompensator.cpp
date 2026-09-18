#include "AutoLevelCompensator.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

namespace {
constexpr double kEpsilon = 1.0e-20;
}

double AutoLevelCompensator::timeCoefficient(
    double sampleRate,
    double milliseconds) noexcept {

    const double fs = std::max(sampleRate, 1000.0);
    const double ms = std::max(milliseconds, 0.01);

    return std::exp(
        -1.0 / (fs * ms * 0.001));
}

void AutoLevelCompensator::prepare(
    double sampleRate) noexcept {

    sampleRate_ =
        (std::isfinite(sampleRate) && sampleRate > 1000.0)
            ? sampleRate
            : 44100.0;

    // Programme energy is learned slowly enough to ignore individual
    // pick transients and palm-mute peaks.
    energyCoefficient_ =
        timeCoefficient(sampleRate_, 500.0);

    // Makeup rises slowly so it cannot act like another compressor.
    // It returns to unity faster when the required compensation falls.
    gainUpCoefficient_ =
        timeCoefficient(sampleRate_, 750.0);

    gainDownCoefficient_ =
        timeCoefficient(sampleRate_, 250.0);

    maxGain_ =
        std::pow(10.0, 1.5 / 20.0);

    // Do not chase amp hiss or numerical residue during silence.
    gateEnergy_ =
        std::pow(10.0, -65.0 / 10.0);

    reset();
}

void AutoLevelCompensator::reset() noexcept {
    inputEnergy_ = 0.0;
    outputEnergy_ = 0.0;
    gain_ = 1.0;
}

double AutoLevelCompensator::currentGainDb() const noexcept {
    return gain_ > 0.0
        ? 20.0 * std::log10(gain_)
        : 0.0;
}

void AutoLevelCompensator::processFrame(
    double referenceLeft,
    double referenceRight,
    double& processedLeft,
    double& processedRight) noexcept {

    if (!std::isfinite(referenceLeft))
        referenceLeft = 0.0;
    if (!std::isfinite(referenceRight))
        referenceRight = 0.0;
    if (!std::isfinite(processedLeft))
        processedLeft = 0.0;
    if (!std::isfinite(processedRight))
        processedRight = 0.0;

    const double inputInstant =
        0.5 * (
            referenceLeft * referenceLeft +
            referenceRight * referenceRight);

    const double outputInstant =
        0.5 * (
            processedLeft * processedLeft +
            processedRight * processedRight);

    inputEnergy_ =
        energyCoefficient_ * inputEnergy_ +
        (1.0 - energyCoefficient_) * inputInstant;

    outputEnergy_ =
        energyCoefficient_ * outputEnergy_ +
        (1.0 - energyCoefficient_) * outputInstant;

    double targetGain = 1.0;

    if (inputEnergy_ > gateEnergy_) {
        const double ratio = std::sqrt(
            (inputEnergy_ + kEpsilon) /
            (outputEnergy_ + kEpsilon));

        // FINISH is primarily subtractive. Auto-level only restores measured
        // loss; it never attenuates a source that happens to become louder.
        targetGain = std::clamp(
            ratio,
            1.0,
            maxGain_);
    }

    const double coefficient =
        targetGain > gain_
            ? gainUpCoefficient_
            : gainDownCoefficient_;

    gain_ =
        coefficient * gain_ +
        (1.0 - coefficient) * targetGain;

    if (std::abs(gain_ - 1.0) < 1.0e-12)
        gain_ = 1.0;

    processedLeft *= gain_;
    processedRight *= gain_;
}

} // namespace HighGainGuitarFinisher::dsp
