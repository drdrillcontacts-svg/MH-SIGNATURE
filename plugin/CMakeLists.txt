cmake_minimum_required(VERSION 3.22)

project(MHSignature VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------ JUCE
include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.15
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# ------------------------------------------------- embedded binary data
juce_add_binary_data(MHSignatureData
    SOURCES
        assets/portrait.jpg)

# ------------------------------------------------------------- plugin
set(MH_FORMATS "VST3;AU;Standalone" CACHE STRING "Formats to build")

juce_add_plugin(MHSignature
    COMPANY_NAME              "Milad Hashemi"
    BUNDLE_ID                 "com.miladhashemi.mhsignature"
    PLUGIN_MANUFACTURER_CODE  Mhsg
    PLUGIN_CODE               Mhs1
    FORMATS                   ${MH_FORMATS}
    PRODUCT_NAME              "MH SIGNATURE"
    DESCRIPTION               "MH SIGNATURE - an all-in-one mixing environment by Milad Hashemi"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    VST3_CATEGORIES           Fx
    AU_MAIN_TYPE              kAudioUnitType_Effect)

target_sources(MHSignature PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Presets.cpp)

target_compile_definitions(MHSignature PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1)

target_link_libraries(MHSignature PRIVATE
    MHSignatureData
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

if (MSVC)
    target_compile_options(MHSignature PRIVATE /utf-8 /MP)
endif()
