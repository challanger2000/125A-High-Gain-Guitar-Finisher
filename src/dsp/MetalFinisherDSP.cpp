#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

void MetalFinisherDSP::prepare(double sampleRate) {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;

    lowCutMixSmoothing_ =
        std::exp(-1.0 / (sampleRate_ * 0.005));

    lowCutFrequencySmoothing_ =
        std::exp(-1.0 / (sampleRate_ * 0.020));

    modeSmoothing_ =
        std::exp(-1.0 / (sampleRate_ * 0.030));

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

    articulation_.prepare(
        sampleRate_,
        AdaptiveBandMode::ArticulationSupport,
        {800.0, 1200.0, 1750.0, 2400.0},
        0.85);

    harshness_.prepare(
        sampleRate_,
        AdaptiveBandMode::Harshness,
        {2800.0, 3600.0, 4500.0, 5600.0},
        1.15);

    fizz_.prepare(
        sampleRate_,
        AdaptiveBandMode::Fizz,
        {6000.0, 7500.0, 9000.0, 11000.0},
        1.0);

    autoLevel_.prepare(sampleRate_);
    room_.prepare(sampleRate_);

    updateModeTargets();
    reset();
}

void MetalFinisherDSP::reset() noexcept {
    for (auto& filter : lowCut_)
        filter.reset();

    lowEnd_.reset();
    body_.reset();
    articulation_.reset();
    harshness_.reset();
    fizz_.reset();
    autoLevel_.reset();
    room_.reset();

    updateModeTargets();
    modeWeights_ = modeWeightTargets_;

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
    const double next =
        std::clamp(
            std::isfinite(normalized)
                ? normalized
                : 0.0,
            0.0,
            1.0);

    const bool wasActive =
        finish_ > 0.0;

    finish_ = next;

    // Do not freeze adaptive IIR/detector state while FINISH is off.
    // Reset once at the transition to exact zero so a later re-enable
    // cannot revive stale filter history from an earlier guitar phrase.
    if (wasActive &&
        finish_ <= 0.0) {

        lowEnd_.reset();
        body_.reset();
        articulation_.reset();
        harshness_.reset();
        fizz_.reset();
        autoLevel_.reset();
    }
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

void MetalFinisherDSP::setMode(double normalized) noexcept {
    modeTarget_ =
        std::clamp(
            std::isfinite(normalized)
                ? normalized
                : 0.0,
            0.0,
            1.0);

    updateModeTargets();
}

void MetalFinisherDSP::updateModeTargets() noexcept {
    const int mode =
        modeTarget_ < 0.25
            ? 0
            : (modeTarget_ < 0.75 ? 1 : 2);

    // Mode 1 is the already validated neutral/modern baseline.
    // Mode 2 preserves more upper-mid bite and adds a little more definition.
    // Mode 3 keeps more mass while controlling the top end more strongly.
    if (mode == 0) {
        modeWeightTargets_ = {
            1.00, // chug
            1.00, // body
            1.00, // articulation
            1.00, // harshness
            1.00  // fizz
        };
    } else if (mode == 1) {
        modeWeightTargets_ = {
            1.10,
            0.80,
            1.35,
            0.55,
            0.60
        };
    } else {
        modeWeightTargets_ = {
            0.65,
            0.55,
            0.30,
            1.30,
            1.35
        };
    }
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
        // Each optimizer zone analyses the same pre-optimizer signal.
        // This avoids order-dependent decisions (for example a body cut
        // changing what the harshness detector sees).
        double lowLeft = baseLeft;
        double lowRight = baseRight;
        double bodyLeft = baseLeft;
        double bodyRight = baseRight;
        double articulationLeft = baseLeft;
        double articulationRight = baseRight;
        double harshLeft = baseLeft;
        double harshRight = baseRight;
        double fizzLeft = baseLeft;
        double fizzRight = baseRight;

        lowEnd_.processFrame(
            lowLeft,
            lowRight);

        body_.processFrame(
            bodyLeft,
            bodyRight);

        articulation_.processFrame(
            articulationLeft,
            articulationRight);

        harshness_.processFrame(
            harshLeft,
            harshRight);

        fizz_.processFrame(
            fizzLeft,
            fizzRight);

        for (std::size_t i = 0;
             i < modeWeights_.size();
             ++i) {
            modeWeights_[i] =
                modeSmoothing_ * modeWeights_[i] +
                (1.0 - modeSmoothing_) *
                    modeWeightTargets_[i];
        }

        const double correctionLeft =
            modeWeights_[0] * (lowLeft - baseLeft) +
            modeWeights_[1] * (bodyLeft - baseLeft) +
            modeWeights_[2] * (articulationLeft - baseLeft) +
            modeWeights_[3] * (harshLeft - baseLeft) +
            modeWeights_[4] * (fizzLeft - baseLeft);

        const double correctionRight =
            modeWeights_[0] * (lowRight - baseRight) +
            modeWeights_[1] * (bodyRight - baseRight) +
            modeWeights_[2] * (articulationRight - baseRight) +
            modeWeights_[3] * (harshRight - baseRight) +
            modeWeights_[4] * (fizzRight - baseRight);

        processedLeft =
            baseLeft +
            correctionLeft * finish_;

        processedRight =
            baseRight +
            correctionRight * finish_;

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
