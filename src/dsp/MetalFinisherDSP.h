#pragma once

#include "AdaptiveBandController.h"
#include "AutoLevelCompensator.h"
#include "Biquad.h"
#include "IndustrialRoom.h"
#include "LowCutMapping.h"

#include <array>

namespace HighGainGuitarFinisher::dsp {

class MetalFinisherDSP {
public:
    void prepare(double sampleRate);
    void reset() noexcept;

    void setFinish(double normalized) noexcept;
    void setLowCut(double normalized) noexcept;
    void setRoomWet(double normalized) noexcept;
    void setRoomDecay(double normalized) noexcept;

    void processFrame(double& left, double& right) noexcept;

    double currentDynamicLowEndReduction() const noexcept {
        return lowEnd_.currentReduction();
    }

    double currentBodyReduction() const noexcept {
        return body_.currentReduction();
    }

    double currentArticulationCorrection() const noexcept {
        return articulation_.currentReduction();
    }

    double currentHarshnessReduction() const noexcept {
        return harshness_.currentReduction();
    }

    double currentFizzReduction() const noexcept {
        return fizz_.currentReduction();
    }

    double currentAutoLevelGainDb() const noexcept {
        return autoLevel_.currentGainDb();
    }

    double detectedLowFrequency() const noexcept {
        return lowEnd_.selectedFrequency();
    }

    double detectedBodyFrequency() const noexcept {
        return body_.selectedFrequency();
    }

    double detectedArticulationFrequency() const noexcept {
        return articulation_.selectedFrequency();
    }

    double detectedHarshnessFrequency() const noexcept {
        return harshness_.selectedFrequency();
    }

    double detectedFizzFrequency() const noexcept {
        return fizz_.selectedFrequency();
    }

private:
    std::array<Biquad, 2> lowCut_ {};

    AdaptiveBandController lowEnd_ {};
    AdaptiveBandController body_ {};
    AdaptiveBandController articulation_ {};
    AdaptiveBandController harshness_ {};
    AdaptiveBandController fizz_ {};
    AutoLevelCompensator autoLevel_ {};
    IndustrialRoom room_ {};

    double sampleRate_ {44100.0};
    double finish_ {0.0};

    double lowCutTarget_ {0.0};
    double lowCutFrequencyHz_ {kLowCutMinimumHz};
    double lowCutMix_ {0.0};
    double lowCutMixSmoothing_ {0.0};
    double lowCutFrequencySmoothing_ {0.0};
    int lowCutCoefficientCountdown_ {0};

    void updateLowCutCoefficients() noexcept;
};

} // namespace HighGainGuitarFinisher::dsp
