cmake_minimum_required(VERSION 3.22)

project(VocalPlus VERSION 1.0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary")

    # Local code signing — use Apple Development cert (not ad-hoc "-") so Gatekeeper keeps trusting the plugin.
    # Pass -DVOCALPLUS_DEVELOPMENT_TEAM=YOUR_TEAM_ID or let build.sh auto-detect from your keychain.
    set(VOCALPLUS_DEVELOPMENT_TEAM "" CACHE STRING "Apple Development Team ID (10 chars, from Xcode Accounts)")
endif()

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(VocalPlus
    COMPANY_NAME "Vocal+"
    BUNDLE_ID com.vocalplus.vocalplus
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VcPl
    PLUGIN_CODE VcP1
    PLUGIN_NAME "Vocal+"
    DESCRIPTION "Vocal+"
    SUPPRESS_AU_PLIST_RESOURCE_USAGE TRUE
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "VocalPlus"
    ICON_BIG Assets/VocalPlus-Icon.png
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
)

juce_generate_juce_header(VocalPlus)

target_sources(VocalPlus
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/DSP/PitchDetector.cpp
        Source/DSP/PhaseVocoderPitchShifter.cpp
        Source/DSP/ResamplePitchShifter.cpp
        Source/DSP/AutoTuneEngine.cpp
        Source/DSP/HarmonyVoice.cpp
        Source/DSP/HarmonyEngine.cpp
        Source/DSP/PresetManager.cpp
        Source/DSP/StemExporter.cpp
)

target_compile_definitions(VocalPlus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VocalPlus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)

juce_add_binary_data(VocalPlusAssets
    SOURCES
        Assets/VocalPlus-Icon.png
)

target_link_libraries(VocalPlus PRIVATE VocalPlusAssets)

juce_add_plugin(EQPlus
    COMPANY_NAME "Vocal+"
    BUNDLE_ID com.vocalplus.eqplus
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VcPl
    PLUGIN_CODE EqP1
    PLUGIN_NAME "EQ+"
    DESCRIPTION "EQ+"
    SUPPRESS_AU_PLIST_RESOURCE_USAGE TRUE
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "EQPlus"
    ICON_BIG Assets/EQPlus-Icon.png
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
)

juce_generate_juce_header(EQPlus)

target_sources(EQPlus
    PRIVATE
        Source/EQPlus/PluginProcessor.cpp
        Source/EQPlus/PluginEditor.cpp
        Source/EQPlus/DSP/EQBand.cpp
        Source/EQPlus/DSP/EQEngine.cpp
        Source/EQPlus/DSP/SpectrumAnalyzer.cpp
        Source/EQPlus/DSP/SmartEQAssistant.cpp
        Source/EQPlus/DSP/EQMatcher.cpp
        Source/EQPlus/UI/EQCurveDisplay.cpp
)

target_compile_definitions(EQPlus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(EQPlus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_binary_data(EQPlusAssets
    SOURCES
        Assets/EQPlus-Icon.png
)

target_link_libraries(EQPlus PRIVATE EQPlusAssets)

juce_add_plugin(TunePlus
    COMPANY_NAME "Vocal+"
    BUNDLE_ID com.vocalplus.tuneplus
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VcPl
    PLUGIN_CODE TnP1
    PLUGIN_NAME "Tune+"
    DESCRIPTION "Tune+ Professional Auto-Tune"
    SUPPRESS_AU_PLIST_RESOURCE_USAGE TRUE
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "TunePlus"
    ICON_BIG Assets/TunePlus-Icon.png
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
)

juce_generate_juce_header(TunePlus)

target_sources(TunePlus
    PRIVATE
        Source/TunePlus/PluginProcessor.cpp
        Source/TunePlus/PluginEditor.cpp
        Source/TunePlus/DSP/VocalCompressor.cpp
        Source/TunePlus/DSP/TunePlusEngine.cpp
        Source/TunePlus/DSP/TunePlusPresetManager.cpp
        Source/DSP/PitchDetector.cpp
        Source/DSP/PhaseVocoderPitchShifter.cpp
        Source/DSP/ResamplePitchShifter.cpp
        Source/DSP/AutoTuneEngine.cpp
        Source/DSP/HarmonyVoice.cpp
        Source/DSP/HarmonyEngine.cpp
        Source/DSP/PresetManager.cpp
        Source/DSP/StemExporter.cpp
)

target_compile_definitions(TunePlus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(TunePlus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_binary_data(TunePlusAssets
    SOURCES
        Assets/TunePlus-Icon.png
)

target_link_libraries(TunePlus PRIVATE TunePlusAssets)

juce_add_plugin(VoxPlus
    COMPANY_NAME "Vocal+"
    BUNDLE_ID com.vocalplus.voxplus
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VcPl
    PLUGIN_CODE VxP1
    PLUGIN_NAME "VOX+"
    DESCRIPTION "VOX+ Vocal Multi-FX"
    SUPPRESS_AU_PLIST_RESOURCE_USAGE TRUE
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "VoxPlus"
    ICON_BIG Assets/VoxPlus-Icon.png
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
)

juce_generate_juce_header(VoxPlus)

target_sources(VoxPlus
    PRIVATE
        Source/VoxPlus/PluginProcessor.cpp
        Source/VoxPlus/PluginEditor.cpp
        Source/VoxPlus/DSP/VoxPlusEngine.cpp
        Source/VoxPlus/DSP/VoxPlusPresetManager.cpp
        Source/TunePlus/DSP/VocalCompressor.cpp
        Source/DSP/PitchDetector.cpp
        Source/DSP/PhaseVocoderPitchShifter.cpp
        Source/DSP/ResamplePitchShifter.cpp
        Source/DSP/AutoTuneEngine.cpp
)

target_compile_definitions(VoxPlus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VoxPlus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_binary_data(VoxPlusAssets
    SOURCES
        Assets/VoxPlus-Icon.png
)

target_link_libraries(VoxPlus PRIVATE VoxPlusAssets)

juce_add_plugin(VocalChangePlus
    COMPANY_NAME "Vocal+"
    BUNDLE_ID com.vocalplus.vocalchangeplus
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VcPl
    PLUGIN_CODE VcC1
    PLUGIN_NAME "VocalChange+"
    DESCRIPTION "VocalChange+ Real-Time Voice Changer"
    SUPPRESS_AU_PLIST_RESOURCE_USAGE TRUE
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "VocalChangePlus"
    ICON_BIG Assets/VocalChangePlus-Icon.png
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
)

juce_generate_juce_header(VocalChangePlus)

target_sources(VocalChangePlus
    PRIVATE
        Source/VocalChangePlus/PluginProcessor.cpp
        Source/VocalChangePlus/PluginEditor.cpp
        Source/VocalChangePlus/DSP/VocalChangeEngine.cpp
        Source/VocalChangePlus/DSP/VocalChangePresetManager.cpp
        Source/DSP/PitchDetector.cpp
        Source/DSP/PhaseVocoderPitchShifter.cpp
        Source/DSP/ResamplePitchShifter.cpp
        Source/DSP/AutoTuneEngine.cpp
)

target_compile_definitions(VocalChangePlus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VocalChangePlus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_binary_data(VocalChangePlusAssets
    SOURCES
        Assets/VocalChangePlus-Icon.png
)

target_link_libraries(VocalChangePlus PRIVATE VocalChangePlusAssets)

juce_add_plugin(VocalAIPlus
    COMPANY_NAME "Vocal+"
    BUNDLE_ID com.vocalplus.vocalaiplus
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE VcPl
    PLUGIN_CODE VaI1
    PLUGIN_NAME "VocalAI+"
    DESCRIPTION "VocalAI+ AI Vocal Mastering"
    SUPPRESS_AU_PLIST_RESOURCE_USAGE TRUE
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "VocalAIPlus"
    ICON_BIG Assets/VocalAIPlus-Icon.png
    AU_MAIN_TYPE kAudioUnitType_Effect
    VST3_CATEGORIES Fx
)

juce_generate_juce_header(VocalAIPlus)

target_sources(VocalAIPlus
    PRIVATE
        Source/VocalAIPlus/PluginProcessor.cpp
        Source/VocalAIPlus/PluginEditor.cpp
        Source/VocalAIPlus/DSP/VocalAIPlusEngine.cpp
        Source/VocalAIPlus/DSP/VocalAnalysisEngine.cpp
        Source/VocalAIPlus/DSP/LocalMasteringAI.cpp
        Source/VocalAIPlus/DSP/VocalAIPlusPresetManager.cpp
        Source/TunePlus/DSP/VocalCompressor.cpp
)

target_compile_definitions(VocalAIPlus
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(VocalAIPlus
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_add_binary_data(VocalAIPlusAssets
    SOURCES
        Assets/VocalAIPlus-Icon.png
)

target_link_libraries(VocalAIPlus PRIVATE VocalAIPlusAssets)

if(TARGET juce_vst3_helper AND TARGET VocalPlus_VST3)
    add_dependencies(VocalPlus_VST3 juce_vst3_helper)
endif()

if(TARGET juce_vst3_helper AND TARGET EQPlus_VST3)
    add_dependencies(EQPlus_VST3 juce_vst3_helper)
endif()

if(TARGET juce_vst3_helper AND TARGET TunePlus_VST3)
    add_dependencies(TunePlus_VST3 juce_vst3_helper)
endif()

if(TARGET juce_vst3_helper AND TARGET VoxPlus_VST3)
    add_dependencies(VoxPlus_VST3 juce_vst3_helper)
endif()

if(TARGET juce_vst3_helper AND TARGET VocalChangePlus_VST3)
    add_dependencies(VocalChangePlus_VST3 juce_vst3_helper)
endif()

if(TARGET juce_vst3_helper AND TARGET VocalAIPlus_VST3)
    add_dependencies(VocalAIPlus_VST3 juce_vst3_helper)
endif()

if(APPLE AND VOCALPLUS_DEVELOPMENT_TEAM)
    set(_vocalplus_entitlements_dir "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

    foreach(_target IN ITEMS VocalPlus_AU VocalPlus_VST3 VocalPlus_Standalone
                            EQPlus_AU EQPlus_VST3 EQPlus_Standalone
                            TunePlus_AU TunePlus_VST3 TunePlus_Standalone
                            VoxPlus_AU VoxPlus_VST3 VoxPlus_Standalone
                            VocalChangePlus_AU VocalChangePlus_VST3 VocalChangePlus_Standalone
                            VocalAIPlus_AU VocalAIPlus_VST3 VocalAIPlus_Standalone)
        if(TARGET ${_target})
            set(_entitlements "${_vocalplus_entitlements_dir}/${_target}.entitlements")
            if(NOT EXISTS "${_entitlements}")
                set(_entitlements "")
            endif()

            set(_sign_flags "")
            if(NOT _target MATCHES "_AU$")
                set(_sign_flags "--options runtime")
            endif()

            set_target_properties(${_target} PROPERTIES
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${VOCALPLUS_DEVELOPMENT_TEAM}"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
                XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
                XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "${_sign_flags}"
            )

            if(_entitlements)
                set_target_properties(${_target} PROPERTIES
                    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${_entitlements}"
                )
            endif()
        endif()
    endforeach()
endif()
