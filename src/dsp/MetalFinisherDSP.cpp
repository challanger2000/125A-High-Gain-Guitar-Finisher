#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>

namespace HighGainGuitarFinisher::dsp {

void MetalFinisherDSP::prepare(double sampleRate) noexcept {
    sampleRate_ = (std::isfinite(sampleRate) && sampleRate > 1000.0)
        ? sampleRate
        : 44100.0;

    updateFilters();
    dynamicLowEnd_.prepare(sampleRate_);
    reset();
}

void MetalFinisherDSP::reset() noexcept {
    for (auto& channel : channels_) {
        channel.highPass.reset();
        channel.lowMid.reset();
    }

    dynamicLowEnd_.reset();
}

void MetalFinisherDSP::setFinish(double normalized) noexcept {
    finish_ = std::clamp(
        std::isfinite(normalized) ? normalized : 0.0,
        0.0,
        1.0);
}

void MetalFinisherDSP::updateFilters() noexcept {
    const auto highPass =
        makeHighPass(sampleRate_, 85.0, 0.7071067811865476);
    const auto lowMid =
        makePeaking(sampleRate_, 300.0, 0.85, -4.0);

    for (auto& channel : channels_) {
        channel.highPass.setCoefficients(highPass);
        channel.lowMid.setCoefficients(lowMid);
    }
}

void MetalFinisherDSP::processFrame(double& left,
                                    double& right) noexcept {
    if (!std::isfinite(left))
        left = 0.0;
    if (!std::isfinite(right))
        right = 0.0;

    if (finish_ <= 0.0)
        return;

    const double dryLeft = left;
    const double dryRight = right;

    double wetLeft =
        channels_[0].lowMid.process(
            channels_[0].highPass.process(left));

    double wetRight =
        channels_[1].lowMid.process(
            channels_[1].highPass.process(right));

    dynamicLowEnd_.processFrame(wetLeft, wetRight);

    left = dryLeft + (wetLeft - dryLeft) * finish_;
    right = dryRight + (wetRight - dryRight) * finish_;
}

} // namespace HighGainGuitarFinisher::dsp
