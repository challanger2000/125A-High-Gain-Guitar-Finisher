set(HIGH_GAIN_GUITAR_FINISHER_SOURCES
    HighGainGuitarFinisherIDs.h
    HighGainGuitarFinisherProcessor.h
    HighGainGuitarFinisherProcessor.cpp
    HighGainGuitarFinisherController.h
    HighGainGuitarFinisherController.cpp
    SteelKnob.h
    SteelKnob.cpp
    SteelPanelView.h
    SteelPanelView.cpp
    BrandLogoView.h
    BrandLogoView.cpp
    dsp/Biquad.h
    dsp/LowCutMapping.h
    dsp/AdaptiveBandController.h
    dsp/AdaptiveBandController.cpp
    dsp/AutoLevelCompensator.h
    dsp/AutoLevelCompensator.cpp
    dsp/IndustrialRoom.h
    dsp/IndustrialRoom.cpp
    dsp/MetalFinisherDSP.h
    dsp/MetalFinisherDSP.cpp
    entry.cpp
)

smtg_add_vst3plugin(
    HighGainGuitarFinisher
    PACKAGE_NAME "125A-High-Gain-Guitar-Finisher"
    SOURCES_LIST ${HIGH_GAIN_GUITAR_FINISHER_SOURCES}
)

target_link_libraries(HighGainGuitarFinisher
    PRIVATE
        sdk
        vstgui_support
)

target_compile_features(HighGainGuitarFinisher PRIVATE cxx_std_17)
hggf_enable_project_warnings(HighGainGuitarFinisher)

if(WIN32)
    target_sources(HighGainGuitarFinisher PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
    )
endif()

smtg_target_add_plugin_resources(
    HighGainGuitarFinisher
    RESOURCES
        ../resource/HighGainGuitarFinisher.uidesc
        ../resource/125A_Logo_Master_FINAL.svg
)

smtg_target_configure_version_file(HighGainGuitarFinisher)
