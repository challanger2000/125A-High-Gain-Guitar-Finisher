# Source code audit — 2026-09-24

Audited branch:
- `v0.3.0-engineering`

Audited production source set:
- all 24 files under `src/`
- CMake warning configuration
- VST3 state/automation/host integration paths
- DSP realtime paths
- GUI/controller ownership paths

Latest confirmed post-audit workflow:
- run #72
- commit `39dd213281ba6cc7c134d81334eb7f313ebdc9c3`
- Steinberg Validator: 47/47 PASS
- internal suite: 14/14 PASS
- realtime benchmark: 0 observed CI overruns
- realtime allocation test: PASS

## Audit categories

The pass explicitly checked:
- undefined-behaviour risks
- pointer/null assumptions
- array and circular-delay bounds
- initialization/reset ordering
- state serialization and migration
- partial-state failure behaviour
- automation interpolation and block boundaries
- mono/stereo handling
- 32/64-bit processing
- finite/NaN/Inf handling
- denormal/subnormal state collapse
- realtime heap allocation
- realtime locks/I/O/logging/string formatting
- unbounded callback work
- GUI/controller lifetime and ownership
- parameter IDs and state compatibility
- plugin/controller version consistency
- sample-rate-dependent coefficient generation
- tail reporting

## Finding 1 — non-transactional failed state loading

### Problem

Processor and controller state readers previously applied early fields while the state stream was still being parsed.

A malformed or truncated later field could therefore make `setState()` / `setComponentState()` return failure after FINISH, ROOM, OUTPUT, BYPASS or LOW CUT had already changed.

### Risk

A corrupted or incomplete project/preset state could leave the live plugin partially changed even though the host was told state restoration failed.

### Fix

Both processor and controller now:
1. read the complete serialized state into local variables;
2. validate and clamp all fields;
3. only commit values after the whole state has parsed successfully.

### Regression

New tests deliberately truncate a v6 state after early fields and verify:
- load returns failure;
- every previously active parameter remains unchanged.

Result: PASS.

## Finding 2 — O(N) ROOM tail clear inside audio processing

### Problem

`IndustrialRoom::CircularDelay::reset()` previously zeroed each delay buffer with `std::fill`.

ROOM can trigger this clear from `processFrame()` after WET reaches zero. At 192 kHz the combined early/late delay storage is roughly 140,000 double slots, so a single callback sample could cause more than 1 MB of synchronous memory writes.

The same reset path can also be reached through broader finisher resets.

### Risk

No heap allocation occurred, but the reset produced avoidable burst work in the realtime thread and could contribute to deadline spikes at very small buffers.

### Fix

Circular delays now use an O(1) logical reset:
- reset write index to zero;
- reset a `samplesSinceReset` validity counter;
- unread overwritten history returns zero until enough new post-reset samples exist.

No buffer clearing occurs in the callback.

### Regression

A new ROOM test:
1. excites the full delay network;
2. disables WET until internal clearing occurs;
3. re-enables WET on silence;
4. verifies no stale pre-reset tail can reappear.

Result: PASS.

## Realtime source-pattern result

After the fix:
- realtime DSP contains no `new` / `new[]`;
- no mutex/locking primitives;
- no file/network I/O;
- no logging/stream formatting;
- no realtime `std::string` work;
- no realtime `std::vector` growth or resize;
- delay allocation remains confined to `prepare()`;
- warmed realtime allocation regression remains 0 allocations.

## Compiler diagnostics

Project code is compiled with:
- MSVC `/W4`
- `/permissive-`

Latest inspected successful build showed no project-source MSVC warnings.

Warnings visible in controller-test builds originate from Steinberg/VSTGUI dependency headers, not 125A source files.

## Reviewed and retained intentionally

No change was justified for:
- adaptive five-zone FINISH architecture;
- Auto Level coefficients;
- MASS curve;
- LOW CUT coefficient topology;
- Mode weights;
- ROOM feedback/diffusion topology;
- six-second tail report;
- sample-offset automation interpolation;
- 5 ms bypass crossfade;
- controller zoom serialization.

Existing measurements and regressions support keeping these designs.

## Residual external validation

No further source-code defect is currently identified from this audit.

Remaining release risk is primarily integration/subjective validation that cannot be proven by static source review alone:
- 125A Plugin Tester editor lifecycle / I/O / torture pass;
- Studio One load/save/reload and automation behaviour;
- actual Windows host GUI open/close/reopen/zoom behaviour;
- final listening on representative high-gain guitar material.

These remain release gates and are not replaced by the source audit.
