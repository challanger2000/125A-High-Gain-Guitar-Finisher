# Measurement strategy

The finisher is not tuned from a single guitar file or by ear alone.

Deterministic DSP measurements are kept separate from production code. Synthetic fixtures establish controlled regressions; fixed real guitar material establishes whether behaviour transfers to the actual product task.

## Release-test enforcement

All pass/fail guards use explicit runtime checks and remain active in Release builds.

A green compile is not a release PASS. Final judgment requires the applicable Steinberg and 125A QA gates.

## FINISH measurements

Current checks include:

- exact transparency at FINISH = 0 when MASS, ROOM and LOW CUT are off;
- exact FINISH 0/50/100 interpolation behaviour;
- adaptive movement across different synthetic guitar signatures;
- mode separation;
- Auto Level bounds, phrase bootstrap, anti-pumping behaviour and silence reset;
- RMS, sample peak and crest behaviour;
- stereo correlation;
- detailed tonal-band energy;
- impulse and zero-lookahead latency;
- stability from 44.1 through 192 kHz.

## MASS measurements

Current checks include:

- exact neutrality at 0%;
- exact linear interpolation at 50%;
- full-curve response at reference frequencies;
- stable sample-rate operation;
- real-audio band/RMS/peak comparisons after the complete FINISH path.

## ROOM metrology

ROOM has dedicated impulse, programme and spectral tests.

Current guards cover:

- first wet-arrival timing across sample rates;
- wet-only level;
- early/mid/late/tail energy windows;
- independent DECAY behaviour;
- low-frequency attenuation relative to the midrange;
- high-frequency attenuation relative to the midrange;
- stereo correlation;
- mono energy retention;
- adaptive ducking and recovery;
- exact zero wet contribution when WET returns to zero;
- finite output and tail clearing.

Recorded numeric reference values must be refreshed only when an intentional ROOM algorithm change is technically justified. They are not regenerated merely to make a regression pass.

## Numerical torture

The engineering branch explicitly tests:

- NaN input;
- positive and negative infinity;
- denormal/subnormal input;
- extremely large but finite samples;
- normal programme recovery after pathological samples.

The DSP must return finite output and must not leave filter/detector state poisoned.

Internal safeguards bound detector energy and recover non-finite filter state. Biquad residual states below the numerical floor are collapsed to exact zero to reduce denormal risk.

## Automation and state

Processor QA must verify:

- stable public parameter IDs;
- legacy state migration;
- safe defaults for parameters absent from older states;
- sample-offset-aware VST3 automation;
- artifact-free bypass transitions with exact unity dry endpoints;
- project/state recall reproducing the intended audio state.

## Realtime measurement

CPU quality is judged by callback-tail behaviour, not average CPU alone.

The next engineering benchmark records, for relevant sample-rate/block-size/control combinations:

- mean block processing time;
- p95;
- p99;
- maximum;
- block deadline;
- overrun count;
- timer/instrumentation overhead.

Timing results from shared CI runners are evidence for regression and gross failures, not a universal end-user CPU guarantee. Shipping decisions should include representative local/host measurements.

## Nonlinear-candidate measurements

No harmonic/analogue stage is accepted without comparative measurements.

For any candidate measure, where applicable:

- harmonic spectrum / THD across input level and frequency;
- IMD;
- asymmetry and DC;
- alias energy;
- 1x/2x/4x targeted oversampling;
- passband/phase impact;
- latency;
- realtime cost;
- level-matched real-guitar comparison.

## Real-world audio

Synthetic fixtures are necessary regression tools but not the final product target.

Real guitar fixtures must cover more than one source/capture family. Comparisons are level matched so louder is not mistaken for better. Subjective listening is used only after objective regressions are clean.
