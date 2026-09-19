#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

void MetalFinisherDSP::prepare(double sampleRate) noexcept {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;

    lowCutMixSmoothing_ =
        std::exp(-1.0 / (sampleRate_ * 0.005));

    lowCutFrequencySmoothing_ =
        std::exp(-1.0 / (sampleRate_ * 0.020));

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
    room_.prepare(sampleRate_);

    reset();
}

void MetalFinisherDSP::reset() noexcept {
    for (auto& filter : lowCut_)
        filter.reset();

    lowEnd_.reset();
    body_.reset();
    harshness_.reset();
    autoLevel_.reset();
    room_.reset();

    const bool lowCutOn =
        lowCutEnabled(lowCutTarget_);

    lowCutMix_ =
        lowCutOn ? 1.0 : 0.0;

    lowCutFrequencyHz_ =
        lowCutOn
            ? lowCutFrequencyFromNormalized(
                lowCutTarget_)
            : kLowCutMinimumHz;

    updateLowCutCoefficients();
    lowCutCoefficientCountdown_ = 0;
}

void MetalFinisherDSP::setFinish(double normalized) noexcept {
    finish_ = std::clamp(
        std::isfinite(normalized) ? normalized : 0.0,
        0.0,
        1.0);
}

void MetalFinisherDSP::setLowCut(double normalized) noexcept {
    lowCutTarget_ =
        std::clamp(
            std::isfinite(normalized)
                ? normalized
                : 0.0,
            0.0,
            1.0);
}

void MetalFinisherDSP::updateLowCutCoefficients() noexcept {
    const auto coefficients =
        makeHighPass(
            sampleRate_,
            lowCutFrequencyHz_,
            0.7071067811865476);

    for (auto& filter : lowCut_)
        filter.setCoefficients(coefficients);
}

void MetalFinisherDSP::setRoomWet(double normalized) noexcept {
    room_.setWetDry(normalized);
}

void MetalFinisherDSP::setRoomDecay(double normalized) noexcept {
    room_.setDecay(normalized);
}

void MetalFinisherDSP::processFrame(
    double& left,
    double& right) noexcept {

    if (!std::isfinite(left))
        left = 0.0;
    if (!std::isfinite(right))
        right = 0.0;

    const bool lowCutOn =
        lowCutEnabled(lowCutTarget_);

    const double targetMix =
        lowCutOn ? 1.0 : 0.0;

    lowCutMix_ =
        lowCutMixSmoothing_ * lowCutMix_ +
        (1.0 - lowCutMixSmoothing_) *
            targetMix;

    if (std::abs(lowCutMix_ - targetMix) < 1.0e-9)
        lowCutMix_ = targetMix;

    if (lowCutOn) {
        const double targetFrequency =
            lowCutFrequencyFromNormalized(
                lowCutTarget_);

        lowCutFrequencyHz_ =
            lowCutFrequencySmoothing_ *
                lowCutFrequencyHz_ +
            (1.0 - lowCutFrequencySmoothing_) *
                targetFrequency;

        if (--lowCutCoefficientCountdown_ <= 0) {
            updateLowCutCoefficients();
            lowCutCoefficientCountdown_ = 16;
        }
    } else {
        lowCutCoefficientCountdown_ = 0;
    }

    const double filteredLeft =
        lowCut_[0].process(left);

    const double filteredRight =
        lowCut_[1].process(right);

    const double baseLeft =
        lowCutMix_ > 0.0
            ? left + (filteredLeft - left) * lowCutMix_
            : left;

    const double baseRight =
        lowCutMix_ > 0.0
            ? right + (filteredRight - right) * lowCutMix_
            : right;

    double processedLeft = baseLeft;
    double processedRight = baseRight;

    if (finish_ > 0.0) {
        double finishLeft = baseLeft;
        double finishRight = baseRight;

        lowEnd_.processFrame(
            finishLeft,
            finishRight);

        body_.processFrame(
            finishLeft,
            finishRight);

        harshness_.processFrame(
            finishLeft,
            finishRight);

        processedLeft =
            baseLeft +
            (finishLeft - baseLeft) *
                finish_;

        processedRight =
            baseRight +
            (finishRight - baseRight) *
                finish_;

        autoLevel_.processFrame(
            baseLeft,
            baseRight,
            processedLeft,
            processedRight);
    } else {
        autoLevel_.reset();
    }

    double roomLeft = 0.0;
    double roomRight = 0.0;

    room_.processFrame(
        processedLeft,
        processedRight,
        roomLeft,
        roomRight);

    left =
        processedLeft +
        roomLeft;

    right =
        processedRight +
        roomRight;
}

} // namespace HighGainGuitarFinisher::dsp
