# DSP plan

## Product target

The 125A High Gain Guitar Finisher is a post-amp/post-cab processor for distorted and high-gain electric guitar.

Its job is not to replace the amp or cabinet. Its job is to move an already usable high-gain tone toward a controlled, dense, modern, mix-ready metal-guitar sound with minimal user interaction.

## User-facing controls

- **FINISH** — main macro from untreated post-amp/cab tone toward the finished metal target.
- **ROOM** — dedicated short industrial/metal guitar ambience. Not a general-purpose reverb.
- **OUTPUT** — final level trim.
- **BYPASS** — unity processing bypass.

## FINISH roadmap

### Stage 1 — implemented

- low-end tightening,
- broad low-mid cleanup around the 300 Hz region,
- exact transparency at FINISH = 0,
- standalone frequency-response smoke tests.

### Stage 2 — implemented

- stereo-linked dynamic low-end dominance detector,
- palm-mute-oriented low-band control,
- attack/release smoothing,
- no broadband gain reduction,
- regression tests for activation and recovery.

### Stage 3 — implemented

- stereo-linked upper-mid dominance detector,
- dynamic harshness/fizz control around 4.8 kHz,
- bounded maximum reduction,
- slower reduction attack to preserve pick definition,
- no permanent low-pass filter,
- regression tests for activation, recovery and 8 kHz preservation.

### Candidate later stages

Only add these after measurement and listening tests demonstrate a real benefit:

1. body stabilization,
2. attack/presence shaping,
3. subtle harmonic cohesion,
4. peak control,
5. loudness-aware compensation.

These are design candidates, not promises that every stage will remain in the final DSP.

## Planned ROOM direction

ROOM should add a short, production-oriented industrial metal ambience rather than an obvious conventional reverb tail.

Candidate ingredients:

- short early reflections,
- dark plate/room hybrid character,
- controlled metallic density,
- high-pass/low-pass shaping in the wet path,
- stereo width limited for mono robustness,
- optional ducking from the dry guitar.

## Engineering rules

- Stereo in / stereo out.
- 32-bit and 64-bit sample processing.
- No hidden global SDK dependency.
- VST3 SDK pinned per repository.
- Parameter state must remain backward-compatible after public test builds.
- Avoid loudness bias when evaluating FINISH.
- Do not add oversampling unless measurements show a nonlinear stage actually benefits from it.
- Measure latency and report it correctly if any future stage introduces latency.
- Keep bypass unity and free of intentional coloration.
- Keep DSP code independent from the VST3 SDK wherever practical.
