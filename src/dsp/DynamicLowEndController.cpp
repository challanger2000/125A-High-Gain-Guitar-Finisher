#include "DynamicLowEndController.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

double DynamicLowEndController::timeCoefficient(double sampleRate,
                                                double milliseconds) noexcept {
    const double fs = std::max(sampleRate, 1000.0);
    const double ms = std::max(milliseconds, 0.01);
    return std::exp(-1.0 / (fs * ms * 0.001));
}

void DynamicLowEndController::prepare(double sampleRate) noexcept {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;

    const auto detectorFilter =
        makeLowPass(sampleRate_, 180.0, 0.7071067811865476);

    for (auto& filter : detectorLowPass_)
        filter.setCoefficients(detectorFilter);

    lowAttack_ = timeCoefficient(sampleRate_, 3.0);
    lowRelease_ = timeCoefficient(sampleRate_, 90.0);
    wideAttack_ = timeCoefficient(sampleRate_, 1.5);
    wideRelease_ = timeCoefficient(sampleRate_, 60.0);
    reductionAttack_ = timeCoefficient(sampleRate_, 5.0);
    reductionRelease_ = timeCoefficient(sampleRate_, 120.0);

    reset();
}

void DynamicLowEndController::reset() noexcept {
    for (auto& filter : detectorLowPass_)
        filter.reset();

    lowEnvelope_ = 0.0;
    wideEnvelope_ = 0.0;
    reduction_ = 0.0;
}

void DynamicLowEndController::processFrame(double& left,
                                           double& right) noexcept {
    if (!std::isfinite(left))
        left = 0.0;
    if (!std::isfinite(right))
        right = 0.0;

    const double lowLeft = detectorLowPass_[0].process(left);
    const double lowRight = detectorLowPass_[1].process(right);

    const double lowMagnitude =
        std::max(std::abs(lowLeft), std::abs(lowRight));
    const double wideMagnitude =
        std::max(std::abs(left), std::abs(right));

    const double lowCoefficient =
        lowMagnitude > lowEnvelope_ ? lowAttack_ : lowRelease_;
    const double wideCoefficient =
        wideMagnitude > wideEnvelope_ ? wideAttack_ : wideRelease_;

    lowEnvelope_ =
        lowCoefficient * lowEnvelope_ +
        (1.0 - lowCoefficient) * lowMagnitude;

    wideEnvelope_ =
        wideCoefficient * wideEnvelope_ +
        (1.0 - wideCoefficient) * wideMagnitude;

    // Level-independent low-frequency dominance detector.
    // Palm-mute-like events tend to push this ratio upward while ordinary
    // mid/high guitar content leaves it below the control region.
    const double dominance =
        lowEnvelope_ / (wideEnvelope_ + 1.0e-12);

    const double activity =
        std::clamp((dominance - 0.38) / 0.34, 0.0, 1.0);

    constexpr double maxLowBandReduction = 0.45;
    const double targetReduction =
        maxLowBandReduction * activity;

    const double reductionCoefficient =
        targetReduction > reduction_
            ? reductionAttack_
            : reductionRelease_;

    reduction_ =
        reductionCoefficient * reduction_ +
        (1.0 - reductionCoefficient) * targetReduction;

    // Subtract only the detected low band. The stereo pair uses one shared
    // reduction value, so left/right image and double-track balance stay stable.
    left -= lowLeft * reduction_;
    right -= lowRight * reduction_;
}

} // namespace HighGainGuitarFinisher::dsp
