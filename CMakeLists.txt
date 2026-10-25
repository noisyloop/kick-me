cmake_minimum_required(VERSION 3.22)

project(KickMe VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(KICKME_COPY_PLUGIN "Copy the built plugin into the system plug-in folder" OFF)
option(KICKME_BUILD_TESTS "Build the offline render tests" ON)

include(FetchContent)
FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.10
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(juce)

set(KICKME_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND KICKME_FORMATS AU)
endif()

juce_add_plugin(KickMe
    COMPANY_NAME "Infinati"
    PLUGIN_MANUFACTURER_CODE Infn
    PLUGIN_CODE InKk
    FORMATS ${KICKME_FORMATS}
    VST3_CATEGORIES Instrument Drum
    PRODUCT_NAME "KickMe"
    PLUGIN_NAME "Kick Me"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD ${KICKME_COPY_PLUGIN}
)

target_sources(KickMe PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(KickMe PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(KickMe
    PRIVATE
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (KICKME_BUILD_TESTS)
    enable_testing()

    add_executable(KickMeTests Tests/RenderTests.cpp)
    target_include_directories(KickMeTests PRIVATE
        Source
        $<TARGET_PROPERTY:KickMe,INCLUDE_DIRECTORIES>)
    target_compile_definitions(KickMeTests PRIVATE
        $<TARGET_PROPERTY:KickMe,COMPILE_DEFINITIONS>)
    target_link_libraries(KickMeTests PRIVATE KickMe)

    add_test(NAME render COMMAND KickMeTests)
endif()
