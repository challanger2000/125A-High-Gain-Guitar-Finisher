# 125A High Gain Guitar Finisher

Post-amp/post-cab processor for shaping distorted and high-gain guitars toward a tighter, cleaner and more mix-ready metal sound.

## Scope

This plugin is intentionally specialized. It is designed for distorted/high-gain electric guitar after an amp/cab stage.

It is **not** intended as an all-purpose processor for clean, acoustic, western or classical guitar.

## Core controls

- **FINISH** — main adaptive metal-finishing macro.
- **LOW CUT 80 Hz** — optional fixed 80 Hz high-pass. Off by default.
- **ROOM** — dedicated short industrial/metal guitar ambience.
- **OUTPUT** — final output trim.
- **BYPASS** — unity bypass.

## Current development status

**0.1.0 development**

The VST3 foundation provides stereo I/O, 32/64-bit processing support, automation/state handling and backward-compatible state migration.

FINISH is adaptive rather than one fixed EQ recipe. The DSP continuously measures the incoming guitar and searches separate low-end, body and harshness regions. Frequency selection is smoothed and stereo-linked.

A bounded internal auto-level stage now compares the signal immediately before and after FINISH. It restores only measured FINISH loudness loss, with slow programme tracking, silence reset and a maximum makeup limit of +1.5 dB. The optional 80 Hz low cut is excluded from that comparison, so the fixed high-pass is never compensated away.

LOW CUT 80 Hz remains a deliberate user choice and is off by default.

ROOM remains intentionally neutral until its dedicated industrial ambience is designed and listening-tested.

## Build and validation

- Windows x64
- Visual Studio 2022
- CMake 3.25+
- Steinberg VST3 SDK **3.8.1**, pinned to v3.8.1_build_84 through CMake FetchContent

The repository does not depend on or modify a global VST3 SDK installation.

The manual **Build Windows VST3** workflow builds the plugin and runs:

- DSP smoke tests,
- adaptive multi-signature fixtures,
- dedicated auto-level tests,
- tonal/RMS/stereo measurements,
- impulse/latency validation.

The workflow runs only when explicitly started, so commits do not consume Actions minutes automatically.

See docs/ARCHITECTURE.md, docs/DSP_PLAN.md and docs/MEASUREMENT_STRATEGY.md for the engineering details.
