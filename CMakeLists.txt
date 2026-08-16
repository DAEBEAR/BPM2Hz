cmake_minimum_required(VERSION 3.22)

project(BPM2Hz VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- CAMBIA questi 4 caratteri (manufacturer/plugin code) prima di distribuire il plugin ---
juce_add_plugin(BPM2Hz
    COMPANY_NAME "MioStudio"
    PLUGIN_MANUFACTURER_CODE Msdo
    PLUGIN_CODE Tcvt
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "BPM2Hz"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(BPM2Hz)

juce_add_binary_data(BPM2HzData SOURCES
    Resources/daebaer_logo.png
)

target_sources(BPM2Hz PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
)

target_compile_definitions(BPM2Hz PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(BPM2Hz PRIVATE
    BPM2HzData
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
