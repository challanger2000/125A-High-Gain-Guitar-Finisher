set(HGGF_DSP_SOURCES
    ../src/dsp/AdaptiveBandController.cpp
    ../src/dsp/AutoLevelCompensator.cpp
    ../src/dsp/IndustrialRoom.cpp
    ../src/dsp/MetalFinisherDSP.cpp
)

add_executable(HighGainGuitarFinisherDSPTests
    dsp_smoke.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(HighGainGuitarFinisherDSPTests
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherDSPTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherDSPTests
)

add_test(
    NAME HighGainGuitarFinisher.DSP.Smoke
    COMMAND HighGainGuitarFinisherDSPTests
)

add_executable(HighGainGuitarFinisherAdaptiveTests
    adaptive_fixture_tests.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(
    HighGainGuitarFinisherAdaptiveTests
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherAdaptiveTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherAdaptiveTests
)

add_test(
    NAME HighGainGuitarFinisher.DSP.Adaptive
    COMMAND HighGainGuitarFinisherAdaptiveTests
)

add_executable(HighGainGuitarFinisherAutoLevelTests
    auto_level_tests.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(
    HighGainGuitarFinisherAutoLevelTests
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherAutoLevelTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherAutoLevelTests
)

add_test(
    NAME HighGainGuitarFinisher.DSP.AutoLevel
    COMMAND HighGainGuitarFinisherAutoLevelTests
)

add_executable(HighGainGuitarFinisherMeasurements
    measurement_suite.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(
    HighGainGuitarFinisherMeasurements
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherMeasurements
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherMeasurements
)

add_test(
    NAME HighGainGuitarFinisher.Measurements
    COMMAND HighGainGuitarFinisherMeasurements
)

add_executable(HighGainGuitarFinisherLatencyImpulse
    latency_impulse.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(
    HighGainGuitarFinisherLatencyImpulse
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherLatencyImpulse
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherLatencyImpulse
)

add_test(
    NAME HighGainGuitarFinisher.LatencyImpulse
    COMMAND HighGainGuitarFinisherLatencyImpulse
)

add_executable(HighGainGuitarFinisherRoomTests
    room_metrology.cpp
    ../src/dsp/IndustrialRoom.cpp
)

target_include_directories(
    HighGainGuitarFinisherRoomTests
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherRoomTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherRoomTests
)

add_test(
    NAME HighGainGuitarFinisher.DSP.Room
    COMMAND HighGainGuitarFinisherRoomTests
)


add_executable(HighGainGuitarOptimizerBehaviorTests
    optimizer_behavior_tests.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(
    HighGainGuitarOptimizerBehaviorTests
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarOptimizerBehaviorTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarOptimizerBehaviorTests
)

add_test(
    NAME HighGainGuitarFinisher.Optimizer.Behavior
    COMMAND HighGainGuitarOptimizerBehaviorTests
)


add_executable(HighGainGuitarFinisherRealtimeBenchmark
    realtime_benchmark.cpp
    ${HGGF_DSP_SOURCES}
)

target_include_directories(
    HighGainGuitarFinisherRealtimeBenchmark
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(
    HighGainGuitarFinisherRealtimeBenchmark
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherRealtimeBenchmark
)

add_test(
    NAME HighGainGuitarFinisher.Realtime.Benchmark
    COMMAND HighGainGuitarFinisherRealtimeBenchmark
)


add_executable(HighGainGuitarFinisherAutomationMathTests
    automation_math_tests.cpp
)

target_include_directories(
    HighGainGuitarFinisherAutomationMathTests
    PRIVATE
        .
        ../src
)

target_compile_features(
    HighGainGuitarFinisherAutomationMathTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherAutomationMathTests
)

add_test(
    NAME HighGainGuitarFinisher.Automation.Math
    COMMAND HighGainGuitarFinisherAutomationMathTests
)


add_executable(HighGainGuitarFinisherStateMigrationTests
    state_migration_tests.cpp
    ../src/HighGainGuitarFinisherProcessor.cpp
    ${HGGF_DSP_SOURCES}
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/memorystream.cpp
)

target_include_directories(
    HighGainGuitarFinisherStateMigrationTests
    PRIVATE
        .
        ../src
        ../src/dsp
)

target_link_libraries(
    HighGainGuitarFinisherStateMigrationTests
    PRIVATE
        sdk
)

target_compile_features(
    HighGainGuitarFinisherStateMigrationTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherStateMigrationTests
)

add_test(
    NAME HighGainGuitarFinisher.State.Migration
    COMMAND HighGainGuitarFinisherStateMigrationTests
)


add_executable(HighGainGuitarFinisherBypassCrossfadeTests
    bypass_crossfade_tests.cpp
)

target_include_directories(
    HighGainGuitarFinisherBypassCrossfadeTests
    PRIVATE
        .
        ../src
)

target_compile_features(
    HighGainGuitarFinisherBypassCrossfadeTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherBypassCrossfadeTests
)

add_test(
    NAME HighGainGuitarFinisher.Bypass.Crossfade
    COMMAND HighGainGuitarFinisherBypassCrossfadeTests
)


add_executable(HighGainGuitarFinisherControllerStateTests
    controller_state_tests.cpp
    ../src/HighGainGuitarFinisherController.cpp
    ../src/SteelKnob.cpp
    ../src/SteelPanelView.cpp
    ../src/BrandLogoView.cpp
    ${vst3sdk_SOURCE_DIR}/public.sdk/source/common/memorystream.cpp
)

target_include_directories(
    HighGainGuitarFinisherControllerStateTests
    PRIVATE
        .
        ../src
        ../src/dsp
)

target_link_libraries(
    HighGainGuitarFinisherControllerStateTests
    PRIVATE
        sdk
        vstgui_support
)

target_compile_features(
    HighGainGuitarFinisherControllerStateTests
    PRIVATE
        cxx_std_17
)

hggf_enable_project_warnings(
    HighGainGuitarFinisherControllerStateTests
)

add_test(
    NAME HighGainGuitarFinisher.Controller.State
    COMMAND HighGainGuitarFinisherControllerStateTests
)
