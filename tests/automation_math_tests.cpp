#include "support/TestSupport.h"
#include "AutomationMath.h"

#include <cmath>
#include <iostream>

using HighGainGuitarFinisher::automation::linearValueAtSample;

namespace {

void requireNear(
    double actual,
    double expected,
    double tolerance = 1.0e-12) {

    HGGF_REQUIRE(
        std::abs(actual - expected) <=
        tolerance);
}

void verifyImplicitMinusOneStart() {
    requireNear(
        linearValueAtSample(
            0, -1, 0.0, 3, 1.0),
        0.25);

    requireNear(
        linearValueAtSample(
            1, -1, 0.0, 3, 1.0),
        0.50);

    requireNear(
        linearValueAtSample(
            2, -1, 0.0, 3, 1.0),
        0.75);

    requireNear(
        linearValueAtSample(
            3, -1, 0.0, 3, 1.0),
        1.00);
}

void verifyBlockEndRamp() {
    requireNear(
        linearValueAtSample(
            0, -1, 0.25, 7, 0.75),
        0.3125);

    requireNear(
        linearValueAtSample(
            7, -1, 0.25, 7, 0.75),
        0.75);
}

void verifyOneSampleJumpRepresentation() {
    requireNear(
        linearValueAtSample(
            4, 4, 0.2, 5, 0.8),
        0.2);

    requireNear(
        linearValueAtSample(
            5, 4, 0.2, 5, 0.8),
        0.8);
}

void verifySafety() {
    requireNear(
        linearValueAtSample(
            3, 4, 0.2, 4, 0.8),
        0.8);

    requireNear(
        linearValueAtSample(
            0, -1,
            std::numeric_limits<double>::quiet_NaN(),
            3, 1.0),
        0.25);

    requireNear(
        linearValueAtSample(
            0, -1, 0.5, 3,
            std::numeric_limits<double>::infinity()),
        0.5);
}

} // namespace

int main() {
    verifyImplicitMinusOneStart();
    verifyBlockEndRamp();
    verifyOneSampleJumpRepresentation();
    verifySafety();

    std::cout
        << "Automation interpolation math tests passed\n";

    return 0;
}
