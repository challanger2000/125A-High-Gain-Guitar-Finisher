add_executable(HighGainGuitarFinisherDSPTests
    dsp_smoke.cpp
    ../src/dsp/DynamicLowEndController.cpp
    ../src/dsp/DynamicHarshnessController.cpp
    ../src/dsp/MetalFinisherDSP.cpp
)

target_include_directories(HighGainGuitarFinisherDSPTests
    PRIVATE
        ../src/dsp
)

target_compile_features(HighGainGuitarFinisherDSPTests PRIVATE cxx_std_17)
hggf_enable_project_warnings(HighGainGuitarFinisherDSPTests)

add_test(
    NAME HighGainGuitarFinisher.DSP.Smoke
    COMMAND HighGainGuitarFinisherDSPTests
)

add_executable(HighGainGuitarFinisherMeasurements
    measurement_suite.cpp
    ../src/dsp/DynamicLowEndController.cpp
    ../src/dsp/DynamicHarshnessController.cpp
    ../src/dsp/MetalFinisherDSP.cpp
)

target_include_directories(HighGainGuitarFinisherMeasurements
    PRIVATE
        .
        ../src/dsp
)

target_compile_features(HighGainGuitarFinisherMeasurements PRIVATE cxx_std_17)
hggf_enable_project_warnings(HighGainGuitarFinisherMeasurements)

add_test(
    NAME HighGainGuitarFinisher.Measurements
    COMMAND HighGainGuitarFinisherMeasurements
)

add_executable(HighGainGuitarFinisherLatencyImpulse
    latency_impulse.cpp
    ../src/dsp/DynamicLowEndController.cpp
    ../src/dsp/DynamicHarshnessController.cpp
    ../src/dsp/MetalFinisherDSP.cpp
)

target_include_directories(HighGainGuitarFinisherLatencyImpulse
    PRIVATE
        ../src/dsp
)

target_compile_features(HighGainGuitarFinisherLatencyImpulse PRIVATE cxx_std_17)
hggf_enable_project_warnings(HighGainGuitarFinisherLatencyImpulse)

add_test(
    NAME HighGainGuitarFinisher.LatencyImpulse
    COMMAND HighGainGuitarFinisherLatencyImpulse
)
