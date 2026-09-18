#include "MetalFinisherDSP.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::dsp::MetalFinisherDSP;

namespace {

constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double sampleRate = 48000.0;

struct SignatureResult {
    double lowFrequency {0.0};
    double bodyFrequency {0.0};
    double harshFrequency {0.0};
    double maxLowReduction {0.0};
};

SignatureResult runSignature(
    double lowFrequency,
    double bodyFrequency,
    double harshFrequency) {

    MetalFinisherDSP dsp;
    dsp.prepare(sampleRate);
    dsp.setFinish(1.0);

    SignatureResult result;

    for (int i = 0;
         i < static_cast<int>(sampleRate * 4.0);
         ++i) {

        const double time =
            static_cast<double>(i) / sampleRate;

        const double palmMuteEnvelope =
            std::fmod(time, 0.25) < 0.060
                ? 1.0
                : 0.15;

        const double x =
            0.22 * std::sin(
                2.0 * pi * 1000.0 * time) +
            palmMuteEnvelope *
                0.45 * std::sin(
                    2.0 * pi * lowFrequency * time) +
            0.30 * std::sin(
                2.0 * pi * bodyFrequency * time) +
            0.22 * std::sin(
                2.0 * pi * harshFrequency * time) +
            0.05 * std::sin(
                2.0 * pi * 9000.0 * time);

        double left = x;
        double right = x;
        dsp.processFrame(left, right);

        assert(std::isfinite(left));
        assert(std::isfinite(right));

        result.maxLowReduction = std::max(
            result.maxLowReduction,
            dsp.currentDynamicLowEndReduction());
    }

    result.lowFrequency =
        dsp.detectedLowFrequency();
    result.bodyFrequency =
        dsp.detectedBodyFrequency();
    result.harshFrequency =
        dsp.detectedHarshnessFrequency();

    return result;
}

} // namespace

int main() {
    const auto signatureA =
        runSignature(90.0, 230.0, 3400.0);

    const auto signatureB =
        runSignature(175.0, 480.0, 6600.0);

    assert(signatureA.maxLowReduction > 0.05);
    assert(signatureB.maxLowReduction > 0.05);

    assert(
        signatureA.lowFrequency <
        signatureB.lowFrequency - 30.0);

    assert(
        signatureA.bodyFrequency <
        signatureB.bodyFrequency - 80.0);

    assert(
        signatureA.harshFrequency <
        signatureB.harshFrequency - 1000.0);

    std::cout
        << "Adaptive fixture tests passed\n"
        << "Signature A detected low/body/harsh: "
        << signatureA.lowFrequency << " / "
        << signatureA.bodyFrequency << " / "
        << signatureA.harshFrequency << " Hz\n"
        << "Signature B detected low/body/harsh: "
        << signatureB.lowFrequency << " / "
        << signatureB.bodyFrequency << " / "
        << signatureB.harshFrequency << " Hz\n";

    return 0;
}
