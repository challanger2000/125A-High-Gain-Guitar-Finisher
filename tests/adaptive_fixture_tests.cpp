#include "support/TestSupport.h"
#include "MetalFinisherDSP.h"

#include <algorithm>
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
    double harshFrequency,
    double fixtureSampleRate = sampleRate) {

    MetalFinisherDSP dsp;
    dsp.prepare(fixtureSampleRate);
    dsp.setFinish(1.0);

    SignatureResult result;

    for (int i = 0;
         i < static_cast<int>(sampleRate * 4.0);
         ++i) {

        const double time =
            static_cast<double>(i) / fixtureSampleRate;

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

        HGGF_REQUIRE(std::isfinite(left));
        HGGF_REQUIRE(std::isfinite(right));

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

    HGGF_REQUIRE(signatureA.maxLowReduction > 0.05);
    HGGF_REQUIRE(signatureB.maxLowReduction > 0.05);

    HGGF_REQUIRE(
        signatureA.lowFrequency <
        signatureB.lowFrequency - 30.0);

    HGGF_REQUIRE(
        signatureA.bodyFrequency <
        signatureB.bodyFrequency - 80.0);

    HGGF_REQUIRE(
        signatureA.harshFrequency <
        signatureB.harshFrequency - 1000.0);

    // The same spectral signature must resolve to the same adaptive
    // regions across the sample rates commonly used by DAWs.
    const auto signature441 =
        runSignature(90.0, 230.0, 3400.0, 44100.0);

    const auto signature480 =
        runSignature(90.0, 230.0, 3400.0, 48000.0);

    const auto signature960 =
        runSignature(90.0, 230.0, 3400.0, 96000.0);

    HGGF_REQUIRE(
        std::abs(
            signature441.lowFrequency -
            signature480.lowFrequency) < 15.0);

    HGGF_REQUIRE(
        std::abs(
            signature960.lowFrequency -
            signature480.lowFrequency) < 15.0);

    HGGF_REQUIRE(
        std::abs(
            signature441.bodyFrequency -
            signature480.bodyFrequency) < 40.0);

    HGGF_REQUIRE(
        std::abs(
            signature960.bodyFrequency -
            signature480.bodyFrequency) < 40.0);

    HGGF_REQUIRE(
        std::abs(
            signature441.harshFrequency -
            signature480.harshFrequency) < 250.0);

    HGGF_REQUIRE(
        std::abs(
            signature960.harshFrequency -
            signature480.harshFrequency) < 250.0);

    HGGF_REQUIRE(
        std::abs(
            signature441.maxLowReduction -
            signature480.maxLowReduction) < 0.08);

    HGGF_REQUIRE(
        std::abs(
            signature960.maxLowReduction -
            signature480.maxLowReduction) < 0.08);

    std::cerr
        << "Sample-rate signature low/body/harsh/reduction 44.1/48/96 kHz:\n"
        << "  44.1: "
        << signature441.lowFrequency << " / "
        << signature441.bodyFrequency << " / "
        << signature441.harshFrequency << " / "
        << signature441.maxLowReduction << "\n"
        << "  48: "
        << signature480.lowFrequency << " / "
        << signature480.bodyFrequency << " / "
        << signature480.harshFrequency << " / "
        << signature480.maxLowReduction << "\n"
        << "  96: "
        << signature960.lowFrequency << " / "
        << signature960.bodyFrequency << " / "
        << signature960.harshFrequency << " / "
        << signature960.maxLowReduction << "\n";

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
