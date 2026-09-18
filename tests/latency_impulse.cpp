#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

int main() {
    constexpr double sampleRate = 48000.0;
    constexpr int count = 8192;

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);

    std::vector<double> output(count, 0.0);

    for (int i = 0; i < count; ++i) {
        double left = i == 0 ? 1.0 : 0.0;
        double right = left;
        dsp.processFrame(left, right);
        output[static_cast<std::size_t>(i)] = left;
        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));
    }

    const auto peakIt = std::max_element(
        output.begin(),
        output.end(),
        [](double a, double b) {
            return std::abs(a) < std::abs(b);
        });

    const auto peakIndex =
        static_cast<int>(std::distance(output.begin(), peakIt));

    // Current IIR/dynamic topology has no look-ahead or buffering latency.
    // An impulse must therefore produce non-zero output immediately.
    HGGF_REQUIRE(output[0] != 0.0);
    HGGF_REQUIRE(peakIndex <= 1);

    double latePeak = 0.0;
    for (int i = 4096; i < count; ++i)
        latePeak = std::max(
            latePeak,
            std::abs(output[static_cast<std::size_t>(i)]));

    HGGF_REQUIRE(latePeak < 1.0e-4);

    std::cout
        << "Impulse/latency test passed\n"
        << "Peak index: " << peakIndex << "\n"
        << "Late tail peak: " << latePeak << "\n";

    return 0;
}
