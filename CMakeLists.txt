cmake_minimum_required(VERSION 3.22)

project(InfinatiKick VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(INFINATI_COPY_PLUGIN "Copy the built plugin into the system plug-in folder" OFF)
option(INFINATI_BUILD_TESTS "Build the offline render tests" ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.10
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(juce)

set(INFINATI_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND INFINATI_FORMATS AU)
endif()

juce_add_plugin(InfinatiKick
    COMPANY_NAME "Infinati"
    PLUGIN_MANUFACTURER_CODE Infn
    PLUGIN_CODE InKk
    FORMATS ${INFINATI_FORMATS}
    VST3_CATEGORIES Instrument Drum
    PRODUCT_NAME "Infinati Kick"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${INFINATI_COPY_PLUGIN}
)

target_sources(InfinatiKick PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(InfinatiKick PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(InfinatiKick
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (INFINATI_BUILD_TESTS)
    enable_testing()

    add_executable(InfinatiKickTests Tests/RenderTests.cpp)
    target_include_directories(InfinatiKickTests PRIVATE
        Source
        $<TARGET_PROPERTY:InfinatiKick,INCLUDE_DIRECTORIES>)
    target_compile_definitions(InfinatiKickTests PRIVATE
        $<TARGET_PROPERTY:InfinatiKick,COMPILE_DEFINITIONS>)
    target_link_libraries(InfinatiKickTests PRIVATE InfinatiKick)

    add_test(NAME render COMMAND InfinatiKickTests)
endif()
