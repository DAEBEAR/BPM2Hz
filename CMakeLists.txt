cmake_minimum_required(VERSION 3.22)

project(TempoConvert VERSION 1.0.0)

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
juce_add_plugin(TempoConvert
    COMPANY_NAME "MioStudio"
    PLUGIN_MANUFACTURER_CODE Msdo
    PLUGIN_CODE Tcvt
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "TempoConvert"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

juce_generate_juce_header(TempoConvert)

target_sources(TempoConvert PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
)

target_compile_definitions(TempoConvert PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(TempoConvert PRIVATE
    juce::juce_audio_utils
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
