#pragma once

#include "AdaptiveBandController.h"
#include "Biquad.h"

#include <array>

namespace HighGainGuitarFinisher::dsp {

class MetalFinisherDSP {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;

    void setFinish(double normalized) noexcept;
    void setLowCut80(bool enabled) noexcept;

    void processFrame(double& left, double& right) noexcept;

    double currentDynamicLowEndReduction() const noexcept {
        return lowEnd_.currentReduction();
    }

    double currentBodyReduction() const noexcept {
        return body_.currentReduction();
    }

    double currentHarshnessReduction() const noexcept {
        return harshness_.currentReduction();
    }

    double detectedLowFrequency() const noexcept {
        return lowEnd_.selectedFrequency();
    }

    double detectedBodyFrequency() const noexcept {
        return body_.selectedFrequency();
    }

    double detectedHarshnessFrequency() const noexcept {
        return harshness_.selectedFrequency();
    }

private:
    std::array<Biquad, 2> lowCut_ {};

    AdaptiveBandController lowEnd_ {};
    AdaptiveBandController body_ {};
    AdaptiveBandController harshness_ {};

    double sampleRate_ {44100.0};
    double finish_ {0.0};

    double lowCutTarget_ {0.0};
    double lowCutMix_ {0.0};
    double lowCutSmoothing_ {0.0};
};

} // namespace HighGainGuitarFinisher::dsp
