# Measurement strategy

The finisher must not be tuned from a single guitar file or by ear alone.

The repository therefore keeps measurement code separate from production DSP and runs deterministic checks on every manually triggered Windows build.

## Borrowed measurement principles

The strategy follows measurement patterns already proven in the 125A repositories:

- **Analysator-Final**
  - null/transparency checks,
  - RMS/peak/crest measurement,
  - stereo correlation and mid/side metrics,
  - detailed tonal-band energy,
  - deterministic behaviour across signal partitions,
  - calibrated loudness/true-peak methodology where those metrics become relevant.

- **125A-MixEngine**
  - impulse-response thinking,
  - explicit latency checks,
  - roundtrip/regression testing,
  - alias/oversampling measurement before adding oversampling.

The High Gain Guitar Finisher does not depend on those repositories at build time. Only the measurement methods are reused.

## Current automated measurements

### 1. Exact transparency

At `FINISH = 0`, left and right channels must null exactly against the input.

### 2. Broad spectral behaviour

A deterministic guitar-like stress signal is analysed in the same detailed bands used by the Analysator measurement approach:

- 20-80 Hz
- 80-250 Hz
- 250-500 Hz
- 500-2000 Hz
- 2000-5000 Hz
- 5000-8000 Hz
- 8000-12000 Hz
- 12000-20000 Hz

These tests use guardrails rather than fixed target curves. Their purpose is to catch unintended broadband damage while the adaptive design evolves.

### 3. RMS / peak / crest

Input and processed output are compared so a new DSP stage cannot hide behind simple loudness bias.

### 4. Stereo integrity

Correlation and mid/side energy are tracked. Stereo-linked adaptive processing must not destabilize the image.

### 5. Dynamic-controller telemetry

Synthetic low-frequency and harshness fixtures verify that detectors activate and recover as intended.

### 6. Impulse / latency

The current topology has no look-ahead or buffering. The impulse test verifies immediate output and guards against accidental latency introduction.

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

Synthetic tests are regression tools, not the product target. Real high-gain guitar files are still required for listening and offline comparison. Measurements on real audio should be interpreted relative to the source rather than forced toward one universal EQ curve.
