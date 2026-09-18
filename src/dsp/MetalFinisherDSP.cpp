#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

void MetalFinisherDSP::prepare(double sampleRate) noexcept {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;

    const auto lowCut =
        makeHighPass(sampleRate_, 80.0, 0.7071067811865476);

    for (auto& filter : lowCut_)
        filter.setCoefficients(lowCut);

    lowCutSmoothing_ =
        std::exp(-1.0 / (sampleRate_ * 0.005));

    lowEnd_.prepare(
        sampleRate_,
        AdaptiveBandMode::LowTransient,
        {85.0, 110.0, 145.0, 180.0},
        2.5);

    body_.prepare(
        sampleRate_,
        AdaptiveBandMode::BodyResonance,
        {220.0, 300.0, 390.0, 500.0},
        0.9);

    harshness_.prepare(
        sampleRate_,
        AdaptiveBandMode::Harshness,
        {3200.0, 4200.0, 5400.0, 6800.0},
        1.0);

    autoLevel_.prepare(sampleRate_);

    reset();
}

void MetalFinisherDSP::reset() noexcept {
    for (auto& filter : lowCut_)
        filter.reset();

    lowEnd_.reset();
    body_.reset();
    harshness_.reset();
    autoLevel_.reset();

    lowCutMix_ = lowCutTarget_;
}

void MetalFinisherDSP::setFinish(double normalized) noexcept {
    finish_ = std::clamp(
        std::isfinite(normalized) ? normalized : 0.0,
        0.0,
        1.0);
}

void MetalFinisherDSP::setLowCut80(bool enabled) noexcept {
    lowCutTarget_ = enabled ? 1.0 : 0.0;
}

void MetalFinisherDSP::processFrame(
    double& left,
    double& right) noexcept {

    if (!std::isfinite(left))
        left = 0.0;
    if (!std::isfinite(right))
        right = 0.0;

    const double filteredLeft =
        lowCut_[0].process(left);

    const double filteredRight =
        lowCut_[1].process(right);

    lowCutMix_ =
        lowCutSmoothing_ * lowCutMix_ +
        (1.0 - lowCutSmoothing_) * lowCutTarget_;

    if (std::abs(lowCutMix_ - lowCutTarget_) < 1.0e-9)
        lowCutMix_ = lowCutTarget_;

    const double baseLeft =
        lowCutMix_ > 0.0
            ? left + (filteredLeft - left) * lowCutMix_
            : left;

    const double baseRight =
        lowCutMix_ > 0.0
            ? right + (filteredRight - right) * lowCutMix_
            : right;

    // FINISH = 0 remains exactly transparent. Resetting the auto-level state
    // here also prevents stale makeup from a previous non-zero FINISH value.
    if (finish_ <= 0.0) {
        autoLevel_.reset();
        left = baseLeft;
        right = baseRight;
        return;
    }

    double wetLeft = baseLeft;
    double wetRight = baseRight;

    lowEnd_.processFrame(wetLeft, wetRight);
    body_.processFrame(wetLeft, wetRight);
    harshness_.processFrame(wetLeft, wetRight);

    left =
        baseLeft + (wetLeft - baseLeft) * finish_;

    right =
        baseRight + (wetRight - baseRight) * finish_;

    // Compare against the post-low-cut reference so the optional fixed
    // 80 Hz filter is never "undone" by makeup gain.
    autoLevel_.processFrame(
        baseLeft,
        baseRight,
        left,
        right);
}

} // namespace HighGainGuitarFinisher::dsp
