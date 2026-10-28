# SPDX-License-Identifier: AGPL-3.0-only
cmake_minimum_required(VERSION 3.22)
project(HolyFoolsPadEngine VERSION 1.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(JUCE_PATH "" CACHE PATH "Optional path to an unmodified JUCE 9.0.4 source tree")
if(NOT JUCE_PATH AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/JUCE/CMakeLists.txt")
    # The complete Corresponding Source release archive builds without a download.
    set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/JUCE" CACHE PATH "JUCE source tree" FORCE)
endif()
if(JUCE_PATH)
    if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
        message(FATAL_ERROR "JUCE_PATH must point to a JUCE 9.0.4 source tree")
    endif()
else()
    include(FetchContent)
    find_package(Git REQUIRED)
    # Shallow checkout by release tag, then verify its immutable commit identity.
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 9.0.4
        GIT_SHALLOW TRUE
        GIT_SUBMODULES ""
        GIT_CONFIG core.autocrlf=false
        # Add JUCE ourselves only after checking its commit and version.
        SOURCE_SUBDIR _verify_before_add)
    FetchContent_MakeAvailable(juce)
    execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
        WORKING_DIRECTORY "${juce_SOURCE_DIR}"
        OUTPUT_VARIABLE juce_commit OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE juce_git_result)
    if(NOT juce_git_result EQUAL 0 OR NOT juce_commit STREQUAL "3fb5cbc0340758596dac39f7c8de47d562b4896e")
        message(FATAL_ERROR "JUCE 9.0.4 checkout does not match the reviewed upstream commit")
    endif()
    set(JUCE_PATH "${juce_SOURCE_DIR}" CACHE PATH "JUCE source tree" FORCE)
endif()
file(READ "${JUCE_PATH}/modules/juce_core/system/juce_StandardHeader.h" juce_version_header)
foreach(component MAJOR_VERSION MINOR_VERSION BUILDNUMBER)
    string(REGEX MATCH "#define JUCE_${component}[ \t]+([0-9]+)" unused "${juce_version_header}")
    set(juce_${component} "${CMAKE_MATCH_1}")
endforeach()
if(NOT "${juce_MAJOR_VERSION}.${juce_MINOR_VERSION}.${juce_BUILDNUMBER}" STREQUAL "9.0.4")
    message(FATAL_ERROR "This release requires JUCE 9.0.4")
endif()
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
    FORMATS Standalone VST3 PRODUCT_NAME "YURODSTVO - Holy Fool's Synth for Arturia MiniLab3"
    # JUCE's factory builds Unicode class info from an ASCII VST3 class name.
    PLUGIN_NAME "YURODSTVO - Holy Fool's Synth for Arturia MiniLab3"
    NEEDS_WEB_BROWSER FALSE NEEDS_CURL FALSE)
# MSBuild's ATL condition cannot parse an apostrophe in a dependency filename.
# Keep the public product name; only the internal static archive uses a plain name.
set_target_properties(Yurodstvo PROPERTIES OUTPUT_NAME "YurodstvoSharedCode")
target_sources(Yurodstvo PRIVATE
    Source/DroneEngine.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/AmbientEffects.cpp Source/ThreeBandEQ.cpp Source/Shimmer.cpp Source/SoundPresets.cpp)
target_compile_definitions(Yurodstvo PUBLIC
    YURO_VERSION="${PROJECT_VERSION}"
    YURO_DISPLAY_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
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
        "${CMAKE_BINARY_DIR}/Yurodstvo_artefacts/$<CONFIG>/VST3/YURODSTVO - Holy Fool's Synth for Arturia MiniLab3.vst3" ${nominal_baseline})
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
