#pragma once

#include "Biquad.h"

#include <array>

namespace HighGainGuitarFinisher::dsp {

class DynamicLowEndController {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void processFrame(double& left, double& right) noexcept;

    double currentReduction() const noexcept {
        return reduction_;
    }

private:
    static double timeCoefficient(double sampleRate,
                                  double milliseconds) noexcept;

    std::array<Biquad, 2> detectorLowPass_ {};

    double sampleRate_ {44100.0};
    double lowEnvelope_ {0.0};
    double wideEnvelope_ {0.0};
    double reduction_ {0.0};

    double lowAttack_ {0.0};
    double lowRelease_ {0.0};
    double wideAttack_ {0.0};
    double wideRelease_ {0.0};
    double reductionAttack_ {0.0};
    double reductionRelease_ {0.0};
};

} // namespace HighGainGuitarFinisher::dsp
