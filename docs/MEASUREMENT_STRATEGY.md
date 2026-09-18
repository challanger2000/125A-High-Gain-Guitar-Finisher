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

Auto Level is reset to unity at FINISH = 0 so previous makeup cannot contaminate a null test or a later dry passage.

### Adaptive multi-signature fixtures

Two deliberately different synthetic high-gain signatures are processed by the same DSP.

The test requires the detected regions to move substantially between them. The present fixture demonstrates movement from approximately:

- low: 85 Hz to 180 Hz,
- body: 220 Hz to 500 Hz,
- harshness: 3.2 kHz to 6.8 kHz.

The exact numbers are not product targets. The test exists to prove that the algorithm is not secretly one fixed 300 Hz / 4.8 kHz recipe.

### Auto-level metrology

Dedicated tests feed the Auto Level with known broadband attenuation at 44.1, 48 and 96 kHz.

They verify:

- recovery of modest measured loss,
- +1.5 dB hard makeup ceiling,
- no gain below unity,
- exact unity after FINISH is set to zero,
- return toward unity across true silence,
- finite output.

The main measurement fixture also requires the final RMS difference to remain within a narrow range around the source while leaving spectral differences measurable.

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

Broadband Auto Level changes absolute band levels but not their relative spectral relationship, so tonal interpretation remains based on the pattern across bands rather than one absolute gain value.

### RMS / peak / crest

Input and output are compared so a processing change cannot hide behind loudness bias.

Peaks are intentionally not matched. A lower processed peak can therefore coexist with RMS/loudness parity.

### Stereo integrity

Correlation and mid/side behaviour are tracked. Stereo-linked adaptive decisions must not destabilize double-tracked guitars.

### Optional 80 Hz low cut

The separate switch is measured independently from FINISH and is excluded from Auto Level's reference comparison.

The current calibrated smoke test expects strong attenuation at 40 Hz while preserving 1 kHz essentially unchanged.

### Impulse / latency

The topology currently has no look-ahead or block buffering. The impulse test verifies immediate output and catches accidental latency introduction.

## Real-world reference measurement

The latest real 0%/100% guitar pair measured approximately:

- pre-Auto-Level RMS difference: -0.55 dB,
- pre-Auto-Level integrated loudness difference: -0.57 LU.

Applying the implemented Auto Level rule offline to the same aligned pair predicts approximately:

- RMS difference: -0.04 dB,
- integrated loudness difference: -0.06 LU,
- maximum makeup: +0.71 dB,
- processed peak still about 0.49 dB below the 0% reference.

This is the intended behaviour: loudness parity without peak restoration.

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
