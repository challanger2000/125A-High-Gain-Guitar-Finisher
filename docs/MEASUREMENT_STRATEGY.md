# Measurement strategy

The finisher must not be tuned from a single guitar file or by ear alone.

The repository therefore keeps measurement code separate from production DSP and runs deterministic checks on every manually triggered Windows build.

## Reused measurement principles

The strategy follows methods already proven in other 125A repositories.

### Analysator-Final

- null/transparency checks,
- RMS/peak/crest measurement,
- stereo correlation and mid/side metrics,
- detailed tonal-band energy,
- deterministic analysis,
- calibrated loudness/true-peak methodology when those metrics become relevant.

### 125A-MixEngine

- impulse-response thinking,
- explicit latency checks,
- roundtrip/regression testing,
- alias and oversampling measurements before adding oversampling.

The High Gain Guitar Finisher does not depend on those repositories at build time. Only their measurement principles are reused.

## Current automated measurements

### Exact transparency

With FINISH at zero and LOW CUT 80 Hz off, left and right must null exactly against the input.

### Adaptive multi-signature fixtures

Two deliberately different synthetic high-gain signatures are processed by the same DSP.

The test requires the detected regions to move substantially between them. The present fixture demonstrates movement from approximately:

- low: 85 Hz to 180 Hz,
- body: 220 Hz to 500 Hz,
- harshness: 3.2 kHz to 6.8 kHz.

The exact numbers are not product targets. The test exists to prove that the algorithm is not secretly one fixed 300 Hz / 4.8 kHz recipe.

### Tonal-band guardrails

A deterministic guitar-like stress signal is measured in:

- 20-80 Hz
- 80-250 Hz
- 250-500 Hz
- 500-2000 Hz
- 2000-5000 Hz
- 5000-8000 Hz
- 8000-12000 Hz
- 12000-20000 Hz

The default adaptive path is explicitly guarded against recreating the old heavy fixed sub/body attenuation.

### RMS / peak / crest

Input and output are compared so a processing change cannot hide behind loudness bias.

### Stereo integrity

Correlation and mid/side behaviour are tracked. Stereo-linked adaptive decisions must not destabilize double-tracked guitars.

### Optional 80 Hz low cut

The separate switch is measured independently from FINISH.

The current calibrated smoke test expects strong attenuation at 40 Hz while preserving 1 kHz essentially unchanged.

### Impulse / latency

The topology currently has no look-ahead or block buffering. The impulse test verifies immediate output and catches accidental latency introduction.

## Later measurements

Before adding nonlinear stages or oversampling:

- harmonic spectrum,
- THD/THD+N where meaningful,
- alias energy above the intended baseband,
- 1x/2x/4x roundtrip comparison,
- exact path latency and host-reported latency.

Before adding ROOM:

- impulse response,
- decay envelope,
- early-reflection timing,
- stereo correlation,
- mono compatibility,
- wet-path spectral balance.

## Real-world audio

Synthetic fixtures are regression tools, not the product target.

Real high-gain guitar files remain necessary for listening and offline comparison. Their measurements should be interpreted relative to each source rather than forced toward one universal EQ curve.
