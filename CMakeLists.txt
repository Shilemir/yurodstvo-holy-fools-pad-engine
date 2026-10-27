cmake_minimum_required(VERSION 3.22)
project(HolyFoolsPadEngine VERSION 0.8.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE" CACHE PATH "Local JUCE framework")
add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/juce")
juce_add_binary_data(YurodstvoAssets NAMESPACE YuroAssets SOURCES
    assets/yurodstvo_icon.png
    assets/fonts/RuslanDisplay-Regular.ttf)
juce_add_plugin(Yurodstvo
    COMPANY_NAME "YURODSTVO" BUNDLE_ID "audio.yurodstvo.droneengine"
    PLUGIN_MANUFACTURER_CODE Yuro PLUGIN_CODE Yd01
    IS_SYNTH TRUE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    FORMATS Standalone VST3 PRODUCT_NAME "YURODSTVO - Holy Fool's Pad Engine"
    # JUCE's factory builds Unicode class info from an ASCII VST3 class name.
    PLUGIN_NAME "YURODSTVO - Holy Fool's Pad Engine"
    NEEDS_WEB_BROWSER FALSE NEEDS_CURL FALSE)
# MSBuild's ATL condition cannot parse an apostrophe in a dependency filename.
# Keep the public product name; only the internal static archive uses a plain name.
set_target_properties(Yurodstvo PROPERTIES OUTPUT_NAME "YurodstvoSharedCode")
target_sources(Yurodstvo PRIVATE
    Source/DroneEngine.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/AmbientEffects.cpp Source/ThreeBandEQ.cpp Source/Shimmer.cpp)
target_compile_definitions(Yurodstvo PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_VST3_EMULATE_MIDI_CC_WITH_PARAMETERS=1)
if(MSVC)
    target_compile_options(Yurodstvo PUBLIC /utf-8)
endif()

target_link_libraries(Yurodstvo PRIVATE juce::juce_audio_utils YurodstvoAssets
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
add_custom_command(TARGET Yurodstvo_Standalone POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL.txt"
        "$<TARGET_FILE_DIR:Yurodstvo_Standalone>/RuslanDisplay-OFL.txt")
add_custom_command(TARGET Yurodstvo_VST3 POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:Yurodstvo_VST3>/../Resources"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL.txt"
        "$<TARGET_FILE_DIR:Yurodstvo_VST3>/../Resources/RuslanDisplay-OFL.txt")
option(YURO_BUILD_TESTS "Build offline technical checks" ON)
if(YURO_BUILD_TESTS)
    enable_testing()
    juce_add_console_app(YurodstvoTests PRODUCT_NAME "Yurodstvo Technical Tests")
    target_sources(YurodstvoTests PRIVATE Tests/TechnicalTests.cpp)
    target_compile_definitions(YurodstvoTests PRIVATE JUCE_PLUGINHOST_VST3=1)
    target_link_libraries(YurodstvoTests PRIVATE Yurodstvo YurodstvoAssets juce::juce_audio_utils)
    add_dependencies(YurodstvoTests Yurodstvo_VST3)
    set(nominal_baseline "${CMAKE_CURRENT_SOURCE_DIR}/artifacts/0.6/VST3/YURODSTVO - Holy Fool's Pad Engine.vst3")
    if(NOT EXISTS "${nominal_baseline}")
        set(nominal_baseline "")
    endif()
    add_test(NAME TechnicalChecks COMMAND YurodstvoTests "${CMAKE_CURRENT_SOURCE_DIR}/artifacts"
        "${CMAKE_BINARY_DIR}/Yurodstvo_artefacts/$<CONFIG>/VST3/YURODSTVO - Holy Fool's Pad Engine.vst3" ${nominal_baseline})
endif()

# JUCE 9.0.4's RC command does not depend on its generated Info.txt. Add the
# dependency locally so incremental builds update Windows names/version too.
if(WIN32)
    foreach(resource_target Yurodstvo YurodstvoTests)
        if(TARGET ${resource_target}_rc_lib)
            get_target_property(resource_dir ${resource_target} JUCE_GENERATED_SOURCES_DIRECTORY)
            get_target_property(resource_info ${resource_target} JUCE_INFO_FILE)
            add_custom_command(OUTPUT "${resource_dir}/${resource_target}_resources.rc"
                APPEND DEPENDS "${resource_info}")
        endif()
    endforeach()
endif()
