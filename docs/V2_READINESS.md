# V2 readiness

Current engineering branch: `v0.3.0-engineering`

Current engineering version string: `0.3.0`

This document records release readiness. It does not declare the branch to be a finished public V2 release.

## Baseline and scope

Published reference baseline:
- 125A High Gain Guitar Finisher v0.2.0

Current engineering work is materially larger than a cosmetic update. The branch is more than one hundred commits ahead of the published baseline and includes substantive DSP, host-behaviour, state, automation and QA changes.

The eventual public version/name must be chosen only when the external release gates below pass.

## Internally verified PASS

Latest confirmed workflow:
- run #72
- commit `39dd213281ba6cc7c134d81334eb7f313ebdc9c3`

Confirmed:
- Steinberg VST3 Validator: 47/47 PASS
- internal CTest suite: 14/14 PASS
- Windows x64 VST3 build artifact produced
- state migration versions 1 through 6
- processor/controller state parity
- parameter text/value round trips for public parameters
- sample-offset-aware automation
- parameter-only no-buffer VST3 flush handling
- 5 ms artifact-safe bypass crossfade
- exact unity dry bypass endpoint
- Mono->Mono and Stereo->Stereo processing
- Mono ROOM collapse parity against Stereo
- offline/realtime sample-exact parity
- 32-bit / 64-bit processing parity within float quantization
- stop/start DSP state reset
- variable processor block sizes from 1 through 1024 samples
- repeated processor setup at 44.1 / 48 / 96 / 192 kHz
- correct 6-second tail reporting
- per-channel silence flags including delayed ROOM tail
- NaN / Inf / extreme finite / denormal input recovery
- zero heap allocations in the warmed realtime DSP path
- zero-lookahead / zero reported latency
- deterministic FINISH and MASS amount-law tests

## Sample-rate consistency

Adaptive FINISH detector signature is identical at 44.1 / 48 / 96 / 192 kHz on the controlled fixture:
- Low: 85 Hz
- Body: 220 Hz
- Harsh: 3600 Hz
- Maximum low reduction: 0.55

Full-path FINISH RMS delta:
- 44.1 kHz: -0.0859 dB
- 48 kHz: -0.0864 dB
- 96 kHz: -0.0823 dB
- 192 kHz: -0.0817 dB

Maximum measured cross-rate spread is approximately 0.005 dB.

LOW CUT at 80 Hz remains approximately -3.0103 dB at all four sample rates.

MASS response also remains effectively invariant across the four sample rates.

## Auto Level

Programme-change anti-pumping test alternates opposing processed-level changes every 125 ms.

Measured gain excursion:
- 44.1 kHz: 0.219711 dB
- 48 kHz: 0.219742 dB
- 96 kHz: 0.219740 dB
- 192 kHz: 0.219740 dB

Maximum sample-to-sample gain step remains extremely small and decreases with higher sample rate.

Decision:
- no Auto Level retuning is justified by current evidence.

## ROOM

Confirmed:
- first reflection approximately 15.7 ms across sample rates
- dense late field with zero empty 5 ms windows in the guarded density ranges
- stable mono compatibility
- bounded ducking and recovery
- dark low/high spectral boundaries
- tail signature stable through 192 kHz

Maximum-decay residual:
- 5.5-6.0 s: approximately -127.64 dBFS RMS
- 6.0-6.4 s: approximately -132.59 dBFS RMS

Decision:
- six-second VST3 tail reporting is conservative and retained.
- no additional diffusion stage is justified.

## Realtime evidence

The benchmark records mean, p95, p99, max, deadline and overrun count after subtracting measured timer overhead.

Shared CI is not a controlled realtime workstation and occasional scheduling spikes occur.

Examples:
- run #67: 0 observed overruns across all 16 rate/block combinations.
- run #70: two isolated shared-runner overruns:
  - 192 kHz / 32 samples: p99 34.1 us versus 166.7 us deadline; one isolated 690 us max spike.
  - 192 kHz / 256 samples: p99 667.8 us versus 1333.3 us deadline; one isolated 8859 us max spike.

The large max spikes are not reflected in the mean/p95/p99 behaviour and vary between identical-code CI runs. They are retained as shared-runner jitter evidence, not hidden.

Run #72, after the source-audit fixes, again observed 0 overruns across all 16 benchmark combinations.

Final release still requires representative local/host testing on the Windows target system.

## Real programme evidence

Two independent high-gain guitar families have been measured:
- TELEFUNKEN Renesans
- Cnoc An Tursa — Bannockburn

Current detector centers, Mode separation, FINISH behaviour, LOW CUT range and MASS design have all been cross-checked against real cabinet/amp material.

The three Modes remain intentionally related:
- Mode 1: Open / Balanced
- Mode 2: Bite / Industrial
- Mode 3: Smooth / Controlled

Real-programme spectral separation supports keeping the current Mode weights.

## Rejected changes

The following were evaluated and deliberately not added:

### Generic nonlinear cohesion / saturation

Rejected because subtle settings did not create enough benefit and stronger settings primarily increased 2.5-10 kHz energy, working against harshness/fizz control.

### Peak catcher / limiter

Rejected because real high-gain reference crest behaviour does not show a repeatable need and the current FINISH path preserves useful transients.

### Additional ROOM diffusion

Rejected because measured late-field density is already continuous and controlled.

### Auto Level retuning

Rejected because anti-pumping and sample-rate measurements are already stable.

### Arbitrary stronger Mode separation

Rejected because current real-programme Mode spacing is musically meaningful and comparable to real alternate cabinet/microphone-family variation.

## External release gates still required

The engineering branch must not be called Final until these pass:

1. 125A Plugin Tester on the built Windows VST3:
   - overall PASS
   - editor open / close / reopen lifecycle
   - I/O and event probe
   - offline / lifecycle / audio torture

2. Studio One host validation and listening:
   - load / save / reload project
   - automation playback
   - bypass switching
   - Mono and Stereo instances where practical
   - representative high-gain guitar material
   - confirm Mode 1/2/3, MASS and ROOM are musically useful at intended ranges
   - confirm no GUI/zoom regression

## Version decision

The current engineering work is technically substantial enough to justify a future V2 rather than another small v0.x maintenance release.

Do not rename or publish as V2 until the external gates above pass.

After those gates pass:
- choose the public V2 version number,
- update version metadata consistently,
- create the release branch/tag/package,
- produce final Gumroad documentation/package,
- keep `main` protected until release integration is intentional.
