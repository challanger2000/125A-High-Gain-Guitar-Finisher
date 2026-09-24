#include "support/TestSupport.h"
#include "BypassCrossfade.h"

#include <algorithm>
#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::BypassCrossfade;

namespace {

void verifyTiming(double sampleRate) {
    BypassCrossfade fade;
    fade.prepare(sampleRate, false);
    fade.setBypassed(true);

    int samples = 0;
    double previous = 0.0;
    double maximumStep = 0.0;

    while (!fade.fullyBypassed() &&
           samples <
               static_cast<int>(
                   sampleRate * 0.020)) {

        const double current =
            fade.advance();

        HGGF_REQUIRE(
            current >= previous);

        maximumStep =
            std::max(
                maximumStep,
                current - previous);

        previous = current;
        ++samples;
    }

    HGGF_REQUIRE(
        fade.fullyBypassed());

    const double milliseconds =
        1000.0 *
        static_cast<double>(samples) /
        sampleRate;

    HGGF_REQUIRE(
        milliseconds >= 4.95 &&
        milliseconds <= 5.10);

    HGGF_REQUIRE(
        maximumStep <=
        1.01 /
        (sampleRate * 0.005));

    fade.setBypassed(false);

    while (fade.mix() > 0.0)
        fade.advance();

    HGGF_REQUIRE(
        fade.mix() == 0.0);
}

void verifyReversalContinuity() {
    BypassCrossfade fade;
    fade.prepare(48000.0, false);
    fade.setBypassed(true);

    double previous = 0.0;

    for (int i = 0; i < 80; ++i)
        previous = fade.advance();

    HGGF_REQUIRE(
        previous > 0.0 &&
        previous < 1.0);

    fade.setBypassed(false);

    const double reversed =
        fade.advance();

    HGGF_REQUIRE(
        reversed < previous);

    HGGF_REQUIRE(
        previous - reversed <
        0.01);

    while (fade.mix() > 0.0)
        fade.advance();

    HGGF_REQUIRE(
        fade.mix() == 0.0);
}

void verifyExactEndpoints() {
    BypassCrossfade fade;

    fade.prepare(
        48000.0,
        true);

    HGGF_REQUIRE(
        fade.mix() == 1.0);
    HGGF_REQUIRE(
        fade.fullyBypassed());

    fade.reset(false);

    HGGF_REQUIRE(
        fade.mix() == 0.0);
    HGGF_REQUIRE(
        !fade.fullyBypassed());
}

} // namespace

int main() {
    for (const double sampleRate :
         {44100.0,
          48000.0,
          96000.0,
          192000.0}) {
        verifyTiming(sampleRate);
    }

    verifyReversalContinuity();
    verifyExactEndpoints();

    std::cout
        << "Bypass crossfade tests passed\n";

    return 0;
}
