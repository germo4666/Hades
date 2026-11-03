cmake_minimum_required(VERSION 3.22)
project(Hades VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

if(APPLE)
  set(HD_FORMATS VST3 AU Standalone)
else()
  set(HD_FORMATS VST3 Standalone)
endif()

juce_add_plugin(Hades
  COMPANY_NAME "GERMO"
  BUNDLE_ID com.germo.hades
  PLUGIN_MANUFACTURER_CODE Germ
  PLUGIN_CODE Hads
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  FORMATS ${HD_FORMATS}
  VST3_CATEGORIES Fx Distortion Reverb
  AU_MAIN_TYPE kAudioUnitType_Effect
  PRODUCT_NAME "HADES")

target_sources(Hades PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_include_directories(Hades PRIVATE Source)

target_compile_definitions(Hades PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(Hades
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

option(HD_TESTS "Build the offline render test" OFF)
if(HD_TESTS)
  juce_add_console_app(HadesTest PRODUCT_NAME "HadesTest")
  target_sources(HadesTest PRIVATE Test/main.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_include_directories(HadesTest PRIVATE Source)
  target_compile_definitions(HadesTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JucePlugin_Name="HADES")
  target_link_libraries(HadesTest PRIVATE juce::juce_audio_utils juce::juce_dsp juce::juce_recommended_config_flags)
endif()
