# DSP plan

## Product target

The 125A High Gain Guitar Finisher is a post-amp/post-cab processor for distorted and high-gain electric guitar.

Its job is not to replace the amp or cabinet. Its job is to move an already usable high-gain tone toward a controlled, dense, modern, mix-ready metal-guitar sound with minimal user interaction.

## User-facing controls

- **FINISH** — main macro from untreated post-amp/cab tone toward the finished metal target.
- **ROOM** — dedicated short industrial/metal guitar ambience. Not a general-purpose reverb.
- **OUTPUT** — final level trim.
- **BYPASS** — true processing bypass at unity.

## Planned FINISH building blocks

The final algorithm may combine several measured/adaptive stages:

1. low-end tightening for palm mutes,
2. low-mid mud control,
3. dynamic resonance suppression,
4. fizz/harshness control,
5. body stabilization,
6. attack/presence shaping,
7. subtle harmonic cohesion,
8. peak control,
9. loudness-aware compensation.

These are design goals, not promises that each stage will remain in the final DSP.

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

## Bootstrap status

Version 0.1.0 contains the VST3 framework, stereo I/O, state handling and the four public parameters.

FINISH and ROOM are intentionally DSP-neutral in the bootstrap build. OUTPUT and BYPASS are functional.
