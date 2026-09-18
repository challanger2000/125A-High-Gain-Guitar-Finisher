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
