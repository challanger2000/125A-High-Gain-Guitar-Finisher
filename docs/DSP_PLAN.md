# DSP plan

## Product target

The 125A High Gain Guitar Finisher is a post-amp/post-cab processor for distorted and high-gain electric guitar.

Its job is not to replace the amp or cabinet. Its job is to move an already usable high-gain tone toward a controlled, dense, modern, mix-ready metal-guitar sound with minimal user interaction.

## User-facing controls

- **FINISH** — adaptive main macro.
- **LOW CUT 80 Hz** — optional conventional high-pass, off by default.
- **ROOM** — short industrial/metal guitar ambience.
- **OUTPUT** — final level trim.
- **BYPASS** — unity processing bypass.

## Implemented adaptive foundation

### Low-end / palm-mute control

The algorithm learns the source's own low-band baseline and reacts to fast low-frequency excess. It searches multiple low-frequency anchors instead of assuming one tuning.

### Body stabilization

The algorithm searches the low-mid/body region for a locally dominant resonance. It does not apply a permanent 300 Hz cut.

### Harshness / fizz control

The algorithm searches several upper-mid regions and reacts to either transient or persistent spectral excess. It no longer assumes that every guitar is harsh at 4.8 kHz.

### FINISH macro

FINISH scales the adaptive wet correction against the measured source. At zero, with LOW CUT off, the audio path remains exactly transparent.

### Internal Auto Level

A slow energy comparator measures the loudness loss caused by FINISH and restores only that loss.

Design boundaries:

- no peak matching,
- no fast gain riding,
- no compensation for LOW CUT 80 Hz,
- maximum makeup +1.5 dB,
- fast return toward unity when the correction requirement falls,
- silence detection resets stale programme history.

The aim is to remove loudness bias when judging FINISH, not to turn the plugin into a compressor or maximizer.

## Why the system remains bounded

Adaptive does not mean unconstrained.

The plugin still uses fixed safety limits for:

- search ranges,
- maximum reduction,
- detector attack/release,
- selection hysteresis,
- frequency crossfade speed,
- Auto Level response and maximum makeup.

These limits prevent unstable self-EQ behaviour and make the processor deterministic enough to test.

## Candidate later stages

Only add these after measurements and listening tests demonstrate a clear benefit:

1. attack/presence shaping,
2. subtle harmonic cohesion,
3. peak control.

Any nonlinear stage must be measured for harmonic structure and aliasing before oversampling is considered.

## Planned ROOM direction

ROOM should add a short, production-oriented industrial metal ambience rather than an obvious conventional reverb tail.

Candidate ingredients:

- short early reflections,
- dark plate/room hybrid character,
- controlled metallic density,
- wet-path high-pass/low-pass shaping,
- restrained stereo width for mono robustness,
- optional ducking from the dry guitar.

## Engineering rules

- Stereo in / stereo out.
- 32-bit and 64-bit sample processing.
- No hidden global SDK dependency.
- VST3 SDK pinned per repository.
- Backward-compatible serialized state.
- Avoid loudness bias when evaluating FINISH.
- Do not add oversampling unless measurements show that a nonlinear stage needs it.
- Measure and report latency if any future stage introduces it.
- Keep bypass unity and free of intentional coloration.
- Keep DSP code independent from the VST3 SDK wherever practical.
