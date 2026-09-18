#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

void MetalFinisherDSP::prepare(double sampleRate) noexcept {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;
    reset();
    updateFilters();
}

void MetalFinisherDSP::reset() noexcept {
    for (auto& channel : channels_) {
        channel.highPass.reset();
        channel.lowMid.reset();
    }
}

void MetalFinisherDSP::setFinish(double normalized) noexcept {
    const double next = std::clamp(
        std::isfinite(normalized) ? normalized : 0.0,
        0.0,
        1.0);

    if (std::abs(next - finish_) < 1.0e-6)
        return;

    finish_ = next;
    updateFilters();
}

void MetalFinisherDSP::updateFilters() noexcept {
    const double cutoffHz = 55.0 + 30.0 * finish_;
    const double lowMidHz = 300.0;
    const double lowMidCutDb = -4.0 * finish_;

    const auto highPass = makeHighPass(sampleRate_, cutoffHz, 0.7071067811865476);
    const auto lowMid = makePeaking(sampleRate_, lowMidHz, 0.85, lowMidCutDb);

    for (auto& channel : channels_) {
        channel.highPass.setCoefficients(highPass);
        channel.lowMid.setCoefficients(lowMid);
    }
}

double MetalFinisherDSP::processSample(int channel, double input) noexcept {
    if (!std::isfinite(input))
        return 0.0;

    if (finish_ <= 0.0)
        return input;

    const int index = std::clamp(channel, 0, 1);
    auto& state = channels_[static_cast<std::size_t>(index)];

    double processed = state.highPass.process(input);
    processed = state.lowMid.process(processed);

    // Preserve exact transparency at FINISH = 0 and make the first stage scale
    // gradually with the macro rather than switching in abruptly.
    return input + (processed - input) * finish_;
}

} // namespace HighGainGuitarFinisher::dsp
