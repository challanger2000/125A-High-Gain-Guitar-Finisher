#pragma once

#include "Biquad.h"

#include <array>

namespace HighGainGuitarFinisher::dsp {

class MetalFinisherDSP {
public:
    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setFinish(double normalized) noexcept;
    double processSample(int channel, double input) noexcept;

private:
    struct ChannelState {
        Biquad highPass;
        Biquad lowMid;
    };

    void updateFilters() noexcept;

    std::array<ChannelState, 2> channels_ {};
    double sampleRate_ {44100.0};
    double finish_ {0.0};
};

} // namespace HighGainGuitarFinisher::dsp
