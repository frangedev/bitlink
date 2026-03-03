cmake_minimum_required(VERSION 3.22)

project(BitLink VERSION 1.0.0 LANGUAGES C CXX)

# JUCE must be present (git submodule). Run: git submodule update --init --recursive
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
  message(FATAL_ERROR "JUCE not found. Run: git submodule update --init --recursive")
endif()
add_subdirectory(JUCE)

juce_add_plugin(BitLink
  VERSION 1.0.0
  COMPANY_NAME "FRANGE"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  PLUGIN_MANUFACTURER_CODE Frng
  PLUGIN_CODE BtLk
  FORMATS AU VST3 CLAP Standalone
  PRODUCT_NAME "BitLink"
)

juce_generate_juce_header(BitLink)

target_sources(BitLink
  PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/AmigaStyle.cpp
    src/CrunchEngine.cpp
    src/VampireSlicer.cpp
    src/CyclicStretch.cpp
    src/SampleTools.cpp
    src/IFF8SVXExport.cpp
    src/WAVExport.cpp
    src/PresetFormat.cpp
)

target_compile_definitions(BitLink
  PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_include_directories(BitLink
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(BitLink
  PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_formats
    juce::juce_dsp
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
