#pragma once

#include "Biquad.h"
#include "DynamicHarshnessController.h"
#include "DynamicLowEndController.h"

#include <array>

namespace HighGainGuitarFinisher::dsp {

class MetalFinisherDSP {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setFinish(double normalized) noexcept;
    void processFrame(double& left, double& right) noexcept;

    double currentDynamicLowEndReduction() const noexcept {
        return dynamicLowEnd_.currentReduction();
    }

    double currentHarshnessReduction() const noexcept {
        return dynamicHarshness_.currentReduction();
    }

private:
    struct ChannelState {
        Biquad highPass;
        Biquad lowMid;
    };

    void updateFilters() noexcept;

    std::array<ChannelState, 2> channels_ {};
    DynamicLowEndController dynamicLowEnd_ {};
    DynamicHarshnessController dynamicHarshness_ {};

    double sampleRate_ {44100.0};
    double finish_ {0.0};
};

} // namespace HighGainGuitarFinisher::dsp
