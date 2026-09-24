#pragma once

#include "Biquad.h"

#include <array>
#include <cstddef>
#include <vector>

namespace HighGainGuitarFinisher::dsp {

class IndustrialRoom {
public:
    void prepare(double sampleRate);
    void reset() noexcept;

    void setWetDry(double normalized) noexcept;
    void setDecay(double normalized) noexcept;

    void processFrame(double inputLeft,
                      double inputRight,
                      double& wetLeft,
                      double& wetRight) noexcept;

    double currentWetDry() const noexcept {
        return wet_;
    }

    double currentDecay() const noexcept {
        return decay_;
    }

    double currentDuckGain() const noexcept {
        return duckGain_;
    }

private:
    class CircularDelay {
    public:
        void prepare(std::size_t maximumDelaySamples);
        void reset() noexcept;
        double read(std::size_t delaySamples) const noexcept;
        void push(double sample) noexcept;

    private:
        std::vector<double> buffer_ {};
        std::size_t writeIndex_ {0};
        std::size_t samplesSinceReset_ {0};
    };

    static double timeCoefficient(double sampleRate,
                                  double milliseconds) noexcept;

    static std::size_t toSamples(double sampleRate,
                                 double milliseconds) noexcept;

    void clearTail() noexcept;

    CircularDelay earlyLeft_ {};
    CircularDelay earlyRight_ {};
    std::array<CircularDelay, 4> late_ {};

    std::array<std::size_t, 6> earlyDelayLeft_ {};
    std::array<std::size_t, 6> earlyDelayRight_ {};
    std::array<std::size_t, 4> lateDelay_ {};

    std::array<Biquad, 2> inputHighPass_ {};
    std::array<Biquad, 2> outputLowPass_ {};
    std::array<Biquad, 2> metalBand_ {};
    std::array<double, 4> dampingState_ {};

    double sampleRate_ {44100.0};

    double wetTarget_ {0.0};
    double wet_ {0.0};

    double decayTarget_ {0.5};
    double decay_ {0.5};

    double controlSmoothing_ {0.0};
    double dampingCoefficient_ {0.0};

    double duckEnvelope_ {0.0};
    double duckAttack_ {0.0};
    double duckRelease_ {0.0};
    double duckGain_ {1.0};

    bool tailCleared_ {true};
};

} // namespace HighGainGuitarFinisher::dsp
