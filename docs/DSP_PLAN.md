# DSP plan

## Product target

The 125A High Gain Guitar Finisher is a post-amp/post-cab processor for distorted and high-gain electric guitar.

Its job is to move an already usable high-gain tone toward a controlled, dense, modern, mix-ready metal-guitar sound with minimal user interaction. It is not an amp or cabinet simulator.

## Current user-facing controls

- FINISH — adaptive main processing amount.
- MODE 1 / 2 / 3 — related optimizer characters.
- MASS — low-end weight / low-mid cleanup character stage.
- ROOM WET — industrial-room amount.
- ROOM DECAY — independent tail control.
- LOW CUT — continuous 45-120 Hz high-pass with true Off.
- OUTPUT — final trim.
- BYPASS — unity processing bypass.

## Current FINISH architecture

FINISH currently provides five coordinated adaptive zones:

- low-end / palm-mute control;
- body-resonance stabilization;
- articulation support when the programme is deficient;
- harshness control;
- fizz control.

The complete full-strength adaptive result is calculated before the FINISH amount interpolation. A bounded bidirectional Auto Level stage reduces loudness bias without peak matching.

The current architecture is intentionally linear in its audio transformations except for the time-varying adaptive control laws. No analogue-style harmonic stage is assumed to be beneficial merely because external circuit/saturation research exists.

## MASS

MASS remains a separate linear, zero-latency stage after FINISH and before ROOM. Its expanded control range was deliberately validated so low settings can be subtle while the upper range can be strongly audible at the user's choice.

Do not replace MASS with a nonlinear bass enhancer without comparative evidence.

## ROOM

ROOM is a purpose-built high-gain-guitar ambience.

Design goals:

- audible space without washing out double-tracked rhythm guitars;
- early industrial-room cues;
- controlled low-frequency accumulation;
- dark upper spectrum;
- useful metallic character without brittle fizz;
- controlled stereo decorrelation with mono retention;
- adaptive ducking behind attacks;
- independent wet and decay controls.

Implementation uses asymmetric early reflections, a four-line FDN, frequency-dependent damping, a restrained metallic band component, wet-path high-pass/low-pass filtering and envelope-driven ducking.

## Engineering baseline for v0.3.0 engineering work

Before adding new sound-shaping mechanisms:

1. keep parameter IDs and state versions backward compatible;
2. preserve exact neutral behaviour where specified;
3. process VST3 automation at supplied sample offsets;
4. prove NaN/Inf/extreme-value recovery;
5. prevent denormal/subnormal state accumulation;
6. establish realtime measurements with mean/p95/p99/max, block deadline and overruns;
7. keep current deterministic and real-audio regressions clean.

## Candidate sonic improvements

The following are experiments, not committed features:

1. frequency- and level-dependent harmonic cohesion;
2. transient-aware peak containment;
3. additional attack/presence shaping;
4. improvements to ROOM density/metallicity if current measurements and real guitar expose a limitation.

A nonlinear candidate must earn inclusion against the current v0.2.0/reference architecture under level-matched comparison.

Before accepting any nonlinear stage, measure where relevant:

- harmonic spectrum and THD versus input level and frequency;
- intermodulation;
- asymmetry and DC;
- transient behaviour;
- alias products at 44.1/48 kHz;
- 1x/2x/4x targeted-oversampling benefit;
- CPU p95/p99/max and block overruns;
- exact latency;
- real-guitar residual and controlled listening.

Target only the nonlinear core for oversampling unless whole-chain oversampling demonstrates a measured end-to-end advantage.

## Realtime rules

- Stereo in / stereo out.
- 32-bit and 64-bit sample processing.
- No hidden global SDK dependency.
- No heap allocation, blocking lock, file/network I/O or logging in process().
- Work must remain bounded.
- Backward-compatible serialized state.
- Avoid loudness bias in A/B evaluation.
- Keep bypass unity.
- Keep DSP independent from the VST3 SDK wherever practical.

## Version decision

The engineering branch is not a release declaration.

A larger marketed generation/version is justified only after a measurable technical and sonic improvement over the published v0.2.0 reference is demonstrated and the full 125A release gate passes.
