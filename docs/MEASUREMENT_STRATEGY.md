# Measurement strategy

The finisher is not tuned from a single guitar file or by ear alone.

The repository keeps deterministic DSP measurements separate from production code and runs them on every manually triggered Windows build.

## Release-test enforcement

All pass/fail guards use explicit runtime checks.

They do not depend on C assert, so Release builds with NDEBUG still fail immediately when a measurement leaves its allowed range.

## FINISH measurements

Current checks include:

- exact transparency at FINISH = 0 when ROOM and LOW CUT are off,
- adaptive movement across different synthetic guitar signatures,
- Auto Level bounds and silence reset,
- RMS, peak and crest behaviour,
- stereo correlation,
- detailed tonal-band energy,
- impulse and zero-lookahead latency.

## ROOM impulse metrology

ROOM has a dedicated wet-only test.

The locally verified design currently measures approximately:

- first reflection: 15.71 ms at 48 kHz,
- maximum wet/dry level: -6.13 dB,
- wet stereo correlation: 0.18,
- mono/stereo wet-energy ratio: -2.29 dB,
- wet low-band energy versus mid-band energy: -10.04 dB,
- wet high-band energy versus mid-band energy: -8.64 dB.

The verified 48 kHz impulse RMS windows are approximately -52.77 dB early, -59.56 dB mid, -66.87 dB late, -75.28 dB very-late, -89.53 dB long-tail and -113.38 dB final-tail.

This is deliberately a short room rather than a conventional long reverb.

## ROOM timing across sample rates

The first-reflection timing is checked at:

- 44.1 kHz,
- 48 kHz,
- 96 kHz.

The test requires the first wet arrival to remain in the intended short-room window independent of sample rate.

## ROOM spectral checks

Wet-only tone measurements enforce:

- low-frequency attenuation relative to the midrange,
- high-frequency attenuation relative to the midrange.

These checks guard the 200 Hz wet high-pass and dark upper-frequency damping without forcing one exact comb-filter response.

## ROOM stereo and mono checks

The wet impulse must decorrelate left and right without becoming anti-phase.

A separate mono-energy guard prevents a superficially wide room from disappearing when summed.

## ROOM state checks

Tests also verify:

- ROOM = 0 produces exactly zero wet output,
- room output stays finite,
- the tail clears after ROOM returns to zero.

## Real guitar reference

Before ROOM was integrated into the plugin, the exact ROOM algorithm was run offline on the latest FINISH 100% guitar render.

At ROOM = 100 it measured approximately:

- wet RMS about -23.3 dB relative to the guitar,
- integrated loudness change about +0.04 LU,
- sample-peak change about +0.44 dB,
- stereo correlation remained essentially unchanged,
- mid/side balance changed by only about 0.07 dB.

This is the intended scale: audible spatial depth without a reverb blanket.

## Later measurements

Before any nonlinear stage or oversampling is added:

- harmonic spectrum,
- THD/THD+N where meaningful,
- alias energy,
- 1x/2x/4x comparison,
- exact path latency.

## Real-world audio

Synthetic fixtures are regression tools, not the final product target.

After the Windows build passes, ROOM must still be rendered on real guitar material at several macro values and judged together with the measurements.
