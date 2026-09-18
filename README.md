# 125A High Gain Guitar Finisher

Post-amp/post-cab processor for shaping distorted and high-gain guitars toward a tighter, cleaner and more mix-ready metal sound.

## Scope

This plugin is intentionally specialized. It is designed for distorted/high-gain electric guitar after an amp/cab stage.

It is **not** intended as an all-purpose processor for clean, acoustic, western or classical guitar.

## Core controls

- **FINISH** — main metal-finishing macro.
- **ROOM** — dedicated short industrial/metal guitar ambience.
- **OUTPUT** — final output trim.
- **BYPASS** — unity bypass.

## Current development status

**0.1.0 development bootstrap**

The VST3 foundation provides stereo I/O, 32/64-bit processing support, automation/state handling and the public parameter structure.

The first measured FINISH stage is implemented as conservative low-end tightening plus broad low-mid cleanup. `FINISH = 0` remains exactly transparent. ROOM is intentionally neutral until its dedicated industrial ambience is designed and listening-tested.

## Build

- Windows x64
- Visual Studio 2022
- CMake 3.25+
- Steinberg VST3 SDK **3.8.1**, pinned to `v3.8.1_build_84` through CMake FetchContent

The repository does not depend on or modify a global VST3 SDK installation.

A manual GitHub Actions workflow is provided under **Build Windows VST3**. It builds the plugin, runs the standalone DSP tests and uploads the VST3 bundle. It runs only when explicitly started, so commits do not consume Actions minutes automatically.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for project structure and [docs/DSP_PLAN.md](docs/DSP_PLAN.md) for the DSP direction.
