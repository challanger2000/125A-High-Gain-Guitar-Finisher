# 125A High Gain Guitar Finisher

Post-amp/post-cab processor for shaping distorted and high-gain guitars toward a tighter, cleaner and more mix-ready metal sound.

## Scope

This plugin is intentionally specialized. It is designed for distorted/high-gain electric guitar after an amp/cab stage.

It is **not** intended as an all-purpose processor for clean, acoustic, western or classical guitar.

## Core controls

- **FINISH** — adaptive metal-finishing macro.
- **ROOM** — short industrial guitar room, not a conventional long reverb.
- **LOW CUT 80 Hz** — optional fixed 80 Hz high-pass, off by default.
- **OUTPUT** — final output trim.
- **BYPASS** — unity bypass.

## Current development status

FINISH uses adaptive low-end, body and harshness analysis rather than one fixed EQ recipe. A bounded internal Auto Level stage removes loudness bias without matching peaks.

ROOM is now implemented as a deliberately short, dark and stereo-safe ambience for high-gain guitars:

- first reflections begin around 11-13 ms,
- a four-line feedback delay network creates a dense short tail,
- the wet path is high-passed to avoid low-end mud,
- upper frequencies are damped for a darker room character,
- stereo width is restrained for mono compatibility,
- fixed ducking keeps the room behind strong pick and palm-mute attacks,
- ROOM controls both wet amount and decay density as one macro.

At ROOM = 0 the room path contributes exactly zero wet signal.

## Validation

The manual **Build Windows VST3** workflow builds the plugin and runs:

- DSP smoke tests,
- adaptive multi-signature fixtures,
- Auto Level tests,
- dedicated ROOM impulse/decay/stereo/spectral metrology,
- tonal/RMS/stereo measurements,
- impulse/latency validation.

All test guards are runtime-enforced in Release builds.

## Build

- Windows x64
- Visual Studio 2022
- CMake 3.25+
- Steinberg VST3 SDK 3.8.1, pinned through CMake FetchContent

See docs/ARCHITECTURE.md, docs/DSP_PLAN.md and docs/MEASUREMENT_STRATEGY.md for engineering details.
