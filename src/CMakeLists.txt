set(HIGH_GAIN_GUITAR_FINISHER_SOURCES
    HighGainGuitarFinisherIDs.h
    HighGainGuitarFinisherProcessor.h
    HighGainGuitarFinisherProcessor.cpp
    HighGainGuitarFinisherController.h
    HighGainGuitarFinisherController.cpp
    dsp/Biquad.h
    dsp/DynamicLowEndController.h
    dsp/DynamicLowEndController.cpp
    dsp/DynamicHarshnessController.h
    dsp/DynamicHarshnessController.cpp
    dsp/MetalFinisherDSP.h
    dsp/MetalFinisherDSP.cpp
    entry.cpp
)

smtg_add_vst3plugin(
    HighGainGuitarFinisher
    PACKAGE_NAME "125A High Gain Guitar Finisher"
    SOURCES_LIST ${HIGH_GAIN_GUITAR_FINISHER_SOURCES}
)

target_link_libraries(HighGainGuitarFinisher
    PRIVATE
        sdk
)

target_compile_features(HighGainGuitarFinisher PRIVATE cxx_std_17)
hggf_enable_project_warnings(HighGainGuitarFinisher)

if(WIN32)
    target_sources(HighGainGuitarFinisher PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
    )
endif()

smtg_target_configure_version_file(HighGainGuitarFinisher)
