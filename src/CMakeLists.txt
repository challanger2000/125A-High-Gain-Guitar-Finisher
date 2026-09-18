set(HIGH_GAIN_GUITAR_FINISHER_SOURCES
    HighGainGuitarFinisherIDs.h
    HighGainGuitarFinisherProcessor.h
    HighGainGuitarFinisherProcessor.cpp
    HighGainGuitarFinisherController.h
    HighGainGuitarFinisherController.cpp
    entry.cpp
)

smtg_add_vst3plugin(HighGainGuitarFinisher
    ${HIGH_GAIN_GUITAR_FINISHER_SOURCES}
)

target_link_libraries(HighGainGuitarFinisher
    PRIVATE
        sdk
)

if(WIN32)
    target_sources(HighGainGuitarFinisher PRIVATE
        ${vst3sdk_SOURCE_DIR}/public.sdk/source/main/dllmain.cpp
    )
endif()

set_target_properties(HighGainGuitarFinisher PROPERTIES
    OUTPUT_NAME "125A High Gain Guitar Finisher"
)

smtg_target_configure_version_file(HighGainGuitarFinisher)
