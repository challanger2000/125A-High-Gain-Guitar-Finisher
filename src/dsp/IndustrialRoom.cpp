#include "IndustrialRoom.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

namespace {

constexpr double kPi =
    3.141592653589793238462643383279502884;

constexpr std::array<double, 4> kEarlyGain {
    0.34,
    0.25,
    -0.18,
    0.14
};

} // namespace

void IndustrialRoom::CircularDelay::prepare(
    std::size_t maximumDelaySamples) {

    buffer_.assign(
        std::max<std::size_t>(
            maximumDelaySamples + 2,
            4),
        0.0);

    writeIndex_ = 0;
}

void IndustrialRoom::CircularDelay::reset() noexcept {
    std::fill(
        buffer_.begin(),
        buffer_.end(),
        0.0);

    writeIndex_ = 0;
}

double IndustrialRoom::CircularDelay::read(
    std::size_t delaySamples) const noexcept {

    if (buffer_.empty())
        return 0.0;

    const std::size_t delay =
        std::clamp<std::size_t>(
            delaySamples,
            1,
            buffer_.size() - 1);

    const std::size_t index =
        (writeIndex_ + buffer_.size() - delay) %
        buffer_.size();

    return buffer_[index];
}

void IndustrialRoom::CircularDelay::push(
    double sample) noexcept {

    if (buffer_.empty())
        return;

    buffer_[writeIndex_] = sample;

    writeIndex_ =
        (writeIndex_ + 1) % buffer_.size();
}

double IndustrialRoom::timeCoefficient(
    double sampleRate,
    double milliseconds) noexcept {

    const double fs =
        std::max(sampleRate, 1000.0);

    const double ms =
        std::max(milliseconds, 0.01);

    return std::exp(
        -1.0 / (fs * ms * 0.001));
}

std::size_t IndustrialRoom::toSamples(
    double sampleRate,
    double milliseconds) noexcept {

    return std::max<std::size_t>(
        1,
        static_cast<std::size_t>(
            std::llround(
                sampleRate *
                milliseconds *
                0.001)));
}

void IndustrialRoom::prepare(
    double sampleRate) {

    sampleRate_ =
        (std::isfinite(sampleRate) &&
         sampleRate > 1000.0)
            ? sampleRate
            : 44100.0;

    earlyDelayLeft_ = {
        toSamples(sampleRate_, 11.3),
        toSamples(sampleRate_, 17.9),
        toSamples(sampleRate_, 26.7),
        toSamples(sampleRate_, 37.1)
    };

    earlyDelayRight_ = {
        toSamples(sampleRate_, 13.1),
        toSamples(sampleRate_, 20.3),
        toSamples(sampleRate_, 29.9),
        toSamples(sampleRate_, 40.7)
    };

    lateDelay_ = {
        toSamples(sampleRate_, 47.9),
        toSamples(sampleRate_, 59.3),
        toSamples(sampleRate_, 71.1),
        toSamples(sampleRate_, 83.7)
    };

    const auto maxEarly =
        std::max(
            *std::max_element(
                earlyDelayLeft_.begin(),
                earlyDelayLeft_.end()),
            *std::max_element(
                earlyDelayRight_.begin(),
                earlyDelayRight_.end()));

    earlyLeft_.prepare(maxEarly);
    earlyRight_.prepare(maxEarly);

    const auto maxLate =
        *std::max_element(
            lateDelay_.begin(),
            lateDelay_.end());

    for (auto& delay : late_)
        delay.prepare(maxLate);

    const auto highPass =
        makeHighPass(
            sampleRate_,
            180.0,
            0.7071067811865476);

    const auto lowPass =
        makeLowPass(
            sampleRate_,
            6200.0,
            0.7071067811865476);

    for (auto& filter : inputHighPass_)
        filter.setCoefficients(highPass);

    for (auto& filter : outputLowPass_)
        filter.setCoefficients(lowPass);

    amountSmoothing_ =
        timeCoefficient(
            sampleRate_,
            25.0);

    dampingCoefficient_ =
        std::exp(
            -2.0 *
            kPi *
            5200.0 /
            sampleRate_);

    duckAttack_ =
        timeCoefficient(
            sampleRate_,
            2.0);

    duckRelease_ =
        timeCoefficient(
            sampleRate_,
            140.0);

    reset();
}

void IndustrialRoom::clearTail() noexcept {
    earlyLeft_.reset();
    earlyRight_.reset();

    for (auto& delay : late_)
        delay.reset();

    for (auto& filter : inputHighPass_)
        filter.reset();

    for (auto& filter : outputLowPass_)
        filter.reset();

    dampingState_.fill(0.0);
    duckEnvelope_ = 0.0;
    tailCleared_ = true;
}

void IndustrialRoom::reset() noexcept {
    clearTail();
    amount_ = amountTarget_;
}

void IndustrialRoom::setAmount(
    double normalized) noexcept {

    amountTarget_ =
        std::clamp(
            std::isfinite(normalized)
                ? normalized
                : 0.0,
            0.0,
            1.0);
}

void IndustrialRoom::processFrame(
    double inputLeft,
    double inputRight,
    double& wetLeft,
    double& wetRight) noexcept {

    if (!std::isfinite(inputLeft))
        inputLeft = 0.0;

    if (!std::isfinite(inputRight))
        inputRight = 0.0;

    amount_ =
        amountSmoothing_ * amount_ +
        (1.0 - amountSmoothing_) *
            amountTarget_;

    if (std::abs(
            amount_ -
            amountTarget_) < 1.0e-9) {
        amount_ = amountTarget_;
    }

    if (amountTarget_ <= 0.0 &&
        amount_ <= 1.0e-7) {

        if (!tailCleared_)
            clearTail();

        wetLeft = 0.0;
        wetRight = 0.0;
        return;
    }

    tailCleared_ = false;

    const double hpLeft =
        inputHighPass_[0].process(
            inputLeft);

    const double hpRight =
        inputHighPass_[1].process(
            inputRight);

    double earlyLeft = 0.0;
    double earlyRight = 0.0;

    for (std::size_t tap = 0;
         tap < kEarlyGain.size();
         ++tap) {

        const bool cross =
            (tap & 1U) != 0U;

        earlyLeft +=
            kEarlyGain[tap] *
            (cross
                ? earlyRight_.read(
                    earlyDelayLeft_[tap])
                : earlyLeft_.read(
                    earlyDelayLeft_[tap]));

        earlyRight +=
            kEarlyGain[tap] *
            (cross
                ? earlyLeft_.read(
                    earlyDelayRight_[tap])
                : earlyRight_.read(
                    earlyDelayRight_[tap]));
    }

    earlyLeft_.push(hpLeft);
    earlyRight_.push(hpRight);

    std::array<double, 4> delayed {};

    for (std::size_t line = 0;
         line < delayed.size();
         ++line) {

        delayed[line] =
            late_[line].read(
                lateDelay_[line]);

        dampingState_[line] =
            dampingCoefficient_ *
                dampingState_[line] +
            (1.0 - dampingCoefficient_) *
                delayed[line];
    }

    const double h0 =
        0.5 * (
            dampingState_[0] +
            dampingState_[1] +
            dampingState_[2] +
            dampingState_[3]);

    const double h1 =
        0.5 * (
            dampingState_[0] -
            dampingState_[1] +
            dampingState_[2] -
            dampingState_[3]);

    const double h2 =
        0.5 * (
            dampingState_[0] +
            dampingState_[1] -
            dampingState_[2] -
            dampingState_[3]);

    const double h3 =
        0.5 * (
            dampingState_[0] -
            dampingState_[1] -
            dampingState_[2] +
            dampingState_[3]);

    const double injectionLeft =
        0.58 * earlyLeft +
        0.10 * hpLeft;

    const double injectionRight =
        0.58 * earlyRight +
        0.10 * hpRight;

    const std::array<double, 4>
        injection {

        injectionLeft +
            0.15 * injectionRight,

        injectionRight -
            0.15 * injectionLeft,

        0.70 * injectionLeft -
            0.25 * injectionRight,

        0.70 * injectionRight +
            0.25 * injectionLeft
    };

    const std::array<double, 4>
        matrix {h0, h1, h2, h3};

    const double feedback =
        0.48 +
        0.18 * amount_;

    for (std::size_t line = 0;
         line < late_.size();
         ++line) {

        late_[line].push(
            injection[line] +
            feedback *
                matrix[line]);
    }

    const double lateLeft =
        0.28 * delayed[0] +
        0.22 * delayed[1] -
        0.18 * delayed[2] +
        0.16 * delayed[3];

    const double lateRight =
        0.16 * delayed[0] -
        0.18 * delayed[1] +
        0.22 * delayed[2] +
        0.28 * delayed[3];

    double rawLeft =
        outputLowPass_[0].process(
            earlyLeft +
            0.90 * lateLeft);

    double rawRight =
        outputLowPass_[1].process(
            earlyRight +
            0.90 * lateRight);

    const double mid =
        0.5 * (
            rawLeft +
            rawRight);

    const double side =
        0.5 * (
            rawLeft -
            rawRight) *
        0.72;

    rawLeft =
        mid + side;

    rawRight =
        mid - side;

    const double magnitude =
        std::max(
            std::abs(inputLeft),
            std::abs(inputRight));

    const double duckCoefficient =
        magnitude > duckEnvelope_
            ? duckAttack_
            : duckRelease_;

    duckEnvelope_ =
        duckCoefficient *
            duckEnvelope_ +
        (1.0 - duckCoefficient) *
            magnitude;

    const double duckActivity =
        std::clamp(
            (duckEnvelope_ - 0.025) /
                0.225,
            0.0,
            1.0);

    const double duckGain =
        1.0 -
        0.42 * duckActivity;

    const double wetGain =
        0.22 *
        amount_ *
        duckGain;

    wetLeft =
        rawLeft * wetGain;

    wetRight =
        rawRight * wetGain;
}

} // namespace HighGainGuitarFinisher::dsp
