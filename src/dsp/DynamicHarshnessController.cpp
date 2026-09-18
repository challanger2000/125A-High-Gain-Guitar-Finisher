#include "DynamicHarshnessController.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

double DynamicHarshnessController::timeCoefficient(
    double sampleRate,
    double milliseconds) noexcept {

    const double fs = std::max(sampleRate, 1000.0);
    const double ms = std::max(milliseconds, 0.01);
    return std::exp(-1.0 / (fs * ms * 0.001));
}

void DynamicHarshnessController::prepare(double sampleRate) noexcept {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;

    const auto harshBand =
        makeBandPass(sampleRate_, 4800.0, 0.85);

    for (auto& filter : harshBand_)
        filter.setCoefficients(harshBand);

    bandAttack_ = timeCoefficient(sampleRate_, 2.0);
    bandRelease_ = timeCoefficient(sampleRate_, 55.0);
    wideAttack_ = timeCoefficient(sampleRate_, 1.5);
    wideRelease_ = timeCoefficient(sampleRate_, 45.0);

    // A slower gain attack than detector attack preserves pick definition.
    reductionAttack_ = timeCoefficient(sampleRate_, 8.0);
    reductionRelease_ = timeCoefficient(sampleRate_, 100.0);

    reset();
}

void DynamicHarshnessController::reset() noexcept {
    for (auto& filter : harshBand_)
        filter.reset();

    bandEnvelope_ = 0.0;
    wideEnvelope_ = 0.0;
    reduction_ = 0.0;
}

void DynamicHarshnessController::processFrame(
    double& left,
    double& right) noexcept {

    if (!std::isfinite(left))
        left = 0.0;
    if (!std::isfinite(right))
        right = 0.0;

    const double bandLeft = harshBand_[0].process(left);
    const double bandRight = harshBand_[1].process(right);

    const double bandMagnitude =
        std::max(std::abs(bandLeft), std::abs(bandRight));
    const double wideMagnitude =
        std::max(std::abs(left), std::abs(right));

    const double bandCoefficient =
        bandMagnitude > bandEnvelope_
            ? bandAttack_
            : bandRelease_;

    const double wideCoefficient =
        wideMagnitude > wideEnvelope_
            ? wideAttack_
            : wideRelease_;

    bandEnvelope_ =
        bandCoefficient * bandEnvelope_ +
        (1.0 - bandCoefficient) * bandMagnitude;

    wideEnvelope_ =
        wideCoefficient * wideEnvelope_ +
        (1.0 - wideCoefficient) * wideMagnitude;

    // Relative detector: react to excessive upper-mid dominance rather than
    // absolute input level. This makes the behaviour much less gain-dependent.
    const double dominance =
        bandEnvelope_ / (wideEnvelope_ + 1.0e-12);

    const double activity =
        std::clamp((dominance - 0.52) / 0.34, 0.0, 1.0);

    constexpr double maxBandReduction = 0.25;
    const double targetReduction =
        maxBandReduction * activity;

    const double reductionCoefficient =
        targetReduction > reduction_
            ? reductionAttack_
            : reductionRelease_;

    reduction_ =
        reductionCoefficient * reduction_ +
        (1.0 - reductionCoefficient) * targetReduction;

    // Subtract only the broad harshness component. One shared reduction value
    // keeps stereo balance stable while preserving the rest of the spectrum.
    left -= bandLeft * reduction_;
    right -= bandRight * reduction_;
}

} // namespace HighGainGuitarFinisher::dsp
