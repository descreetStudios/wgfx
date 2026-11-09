# WgfxShaders.cmake
#
# Build-time shader toolchain for WGFX.
#
# WGFX does not ship shader sources. Projects provide their own bgfx `.sc`
# shaders and compile them to bgfx shader binaries with bgfx's `shaderc`.
# This module wires that compiler into the CMake build and exposes the
# resulting `.bin` files so they can be loaded at runtime with
# `wgfx::Shader::load`.
#
# The module is included by the top-level CMakeLists.txt and can be used from
# any subdirectory afterwards.
#
#   wgfx_add_shader(
#       OUTPUT vs_main
#       SOURCE shaders/vs_main.sc
#       STAGE vertex
#       VARYINGDEF shaders/varying.def.sc)
#
#   wgfx_target_add_shaders(my_target OUTPUTS ${vs_main})
#
# The output defaults to
#   ${WGFX_SHADER_OUTPUT_DIR}/<platform>/<source-name>.bin

include_guard(GLOBAL)

set(WGFX_SHADERC_EXECUTABLE "" CACHE FILEPATH
    "Path to the bgfx shaderc executable. Empty auto-detects a built or installed shaderc.")
set(WGFX_SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders" CACHE PATH
    "Directory where WGFX writes compiled shader binaries.")
set(WGFX_BGFX_SHADER_INCLUDE_DIR "" CACHE PATH
    "Directory containing bgfx_shader.sh and bgfx_compute.sh. Empty auto-detects bgfx.")
set(WGFX_SHADER_PLATFORM "" CACHE STRING
    "Default shaderc target platform. Empty follows the host platform.")
set(WGFX_SHADER_PROFILE "" CACHE STRING
    "Default shaderc profile. Empty follows the platform default.")

# Resolves the shaderc command. Returns a plain path, an absolute path or a
# generator expression when shaderc is a build target.
function(wgfx_shaderc_command out_var)
    if(WGFX_SHADERC_EXECUTABLE)
        if(NOT EXISTS "${WGFX_SHADERC_EXECUTABLE}")
            message(FATAL_ERROR
                "wgfx: WGFX_SHADERC_EXECUTABLE points to "
                "'${WGFX_SHADERC_EXECUTABLE}', which does not exist")
        endif()
        set(${out_var} "${WGFX_SHADERC_EXECUTABLE}" PARENT_SCOPE)
        return()
    endif()

    if(TARGET bgfx::shaderc)
        set(${out_var} "$<TARGET_FILE:bgfx::shaderc>" PARENT_SCOPE)
        return()
    endif()

    if(TARGET shaderc)
        set(${out_var} "$<TARGET_FILE:shaderc>" PARENT_SCOPE)
        return()
    endif()

    find_program(WGFX_SHADERC_PROGRAM NAMES shaderc)
    if(WGFX_SHADERC_PROGRAM)
        set(${out_var} "${WGFX_SHADERC_PROGRAM}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

# Sets out_var to TRUE when a shaderc command is available.
function(wgfx_shaderc_available out_var)
    wgfx_shaderc_command(_command)
    if(_command)
        set(${out_var} TRUE PARENT_SCOPE)
    else()
        set(${out_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

# Fails configuration when shaderc was explicitly requested but could not be
# found. Use this after bgfx has been configured, so a shaderc target exists.
function(wgfx_shaderc_require)
    wgfx_shaderc_available(_available)
    if(NOT _available)
        message(FATAL_ERROR
            "wgfx: WGFX_WITH_SHADERC=ON but shaderc was not found. "
            "Point WGFX_SHADERC_EXECUTABLE at a shaderc binary, or configure "
            "with -DWGFX_WITH_SHADERC=OFF to disable shader compilation.")
    endif()
endfunction()

# Sets out_var to the real shaderc target name, or an empty string.
function(wgfx_shaderc_target out_var)
    if(TARGET bgfx::shaderc OR TARGET shaderc)
        set(${out_var} shaderc PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

# Sets out_var to shaderc's platform name for the host.
function(wgfx_shader_host_platform out_var)
    if(EMSCRIPTEN)
        set(_platform emscripten)
    elseif(WIN32)
        set(_platform windows)
    elseif(APPLE)
        if(IOS)
            set(_platform ios)
        else()
            set(_platform osx)
        endif()
    elseif(ANDROID)
        set(_platform android)
    elseif(UNIX)
        set(_platform linux)
    else()
        set(_platform windows)
    endif()

    set(${out_var} "${_platform}" PARENT_SCOPE)
endfunction()

# Sets out_var to the default profile for a platform.
function(wgfx_shader_default_profile out_var platform)
    if(platform STREQUAL "windows")
        set(_profile s_5_0)
    elseif(platform STREQUAL "osx" OR platform STREQUAL "ios")
        set(_profile metal)
    elseif(platform STREQUAL "emscripten")
        set(_profile 300_es)
    elseif(platform STREQUAL "android")
        set(_profile spirv)
    else()
        set(_profile spirv)
    endif()

    set(${out_var} "${_profile}" PARENT_SCOPE)
endfunction()

# Compiles one shader source into a bgfx shader binary.
#
# Required arguments:
#   OUTPUT     Variable that receives the generated .bin path.
#   SOURCE     Shader source file (.sc).
#   STAGE      vertex, fragment or compute.
#
# Optional arguments:
#   PLATFORM   shaderc --platform (default: host, see WGFX_SHADER_PLATFORM).
#   PROFILE    shaderc --profile (default: platform default).
#   NAME       Output file name without extension (default: source file name).
#   OUTPUT_DIR Output directory (default: WGFX_SHADER_OUTPUT_DIR/<platform>).
#   VARYINGDEF varying.def.sc used by shaderc.
#   INCLUDES   Semicolon-separated include directories passed with -i.
#   DEFINES    Semicolon-separated preprocessor defines.
#   DEPENDS    Extra files that should trigger a rebuild.
function(wgfx_add_shader)
    cmake_parse_arguments(ARG
        ""
        "OUTPUT;SOURCE;STAGE;PLATFORM;PROFILE;NAME;OUTPUT_DIR;VARYINGDEF"
        "INCLUDES;DEFINES;DEPENDS"
        ${ARGN})

    if(NOT ARG_OUTPUT)
        message(FATAL_ERROR "wgfx_add_shader: OUTPUT is required")
    endif()
    if(NOT ARG_SOURCE)
        message(FATAL_ERROR "wgfx_add_shader: SOURCE is required")
    endif()
    if(NOT ARG_STAGE)
        message(FATAL_ERROR "wgfx_add_shader: STAGE is required")
    endif()
    if(NOT ARG_STAGE MATCHES "^(vertex|fragment|compute)$")
        message(FATAL_ERROR
            "wgfx_add_shader: STAGE must be one of vertex, fragment or compute, "
            "got '${ARG_STAGE}'")
    endif()

    wgfx_shaderc_command(_shaderc)
    if(NOT _shaderc)
        message(FATAL_ERROR
            "wgfx_add_shader: shaderc was not found. Configure with "
            "-DWGFX_WITH_SHADERC=ON, point -DWGFX_SHADERC_EXECUTABLE at a "
            "shaderc binary, or do not compile shaders.")
    endif()

    get_filename_component(_source "${ARG_SOURCE}" ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    if(NOT EXISTS "${_source}")
        message(FATAL_ERROR
            "wgfx_add_shader: source '${ARG_SOURCE}' does not exist "
            "(resolved to '${_source}')")
    endif()

    if(ARG_PLATFORM)
        set(_platform "${ARG_PLATFORM}")
    elseif(WGFX_SHADER_PLATFORM)
        set(_platform "${WGFX_SHADER_PLATFORM}")
    else()
        wgfx_shader_host_platform(_platform)
    endif()

    if(ARG_PROFILE)
        set(_profile "${ARG_PROFILE}")
    elseif(WGFX_SHADER_PROFILE)
        set(_profile "${WGFX_SHADER_PROFILE}")
    else()
        wgfx_shader_default_profile(_profile "${_platform}")
    endif()

    if(ARG_NAME)
        set(_name "${ARG_NAME}")
    else()
        get_filename_component(_name "${_source}" NAME_WE)
    endif()

    if(ARG_OUTPUT_DIR)
        set(_output_dir "${ARG_OUTPUT_DIR}")
    else()
        set(_output_dir "${WGFX_SHADER_OUTPUT_DIR}/${_platform}")
    endif()
    get_filename_component(_output_dir "${_output_dir}" ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

    set(_output "${_output_dir}/${_name}.bin")

    set(_varyingdef_args)
    if(ARG_VARYINGDEF)
        get_filename_component(_varyingdef "${ARG_VARYINGDEF}" ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        if(NOT EXISTS "${_varyingdef}")
            message(FATAL_ERROR
                "wgfx_add_shader: varying def '${ARG_VARYINGDEF}' does not exist "
                "(resolved to '${_varyingdef}')")
        endif()
        list(APPEND _varyingdef_args --varyingdef "${_varyingdef}")
    endif()

    set(_include_args)
    if(WGFX_BGFX_SHADER_INCLUDE_DIR)
        list(APPEND _include_args -i "${WGFX_BGFX_SHADER_INCLUDE_DIR}")
    endif()
    foreach(_include IN LISTS ARG_INCLUDES)
        get_filename_component(_include "${_include}" ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND _include_args -i "${_include}")
    endforeach()

    set(_define_args)
    if(ARG_DEFINES)
        list(JOIN ARG_DEFINES ";" _defines)
        list(APPEND _define_args --define "${_defines}")
    endif()

    set(_depends "${_source}" ${ARG_DEPENDS})
    if(ARG_VARYINGDEF)
        list(APPEND _depends "${_varyingdef}")
    endif()

    add_custom_command(
        OUTPUT "${_output}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${_output_dir}"
        COMMAND "${_shaderc}"
            -f "${_source}"
            -o "${_output}"
            --type "${ARG_STAGE}"
            --platform "${_platform}"
            --profile "${_profile}"
            ${_varyingdef_args}
            ${_include_args}
            ${_define_args}
        DEPENDS ${_depends}
        COMMENT "Compiling shader ${_name} (${_platform}/${_profile})"
        VERBATIM)

    set(${ARG_OUTPUT} "${_output}" PARENT_SCOPE)
endfunction()

# Makes `target` depend on compiled shader outputs.
#
#   wgfx_target_add_shaders(my_target OUTPUTS ${vs_main} ${fs_main})
function(wgfx_target_add_shaders target)
    cmake_parse_arguments(ARG "" "" "OUTPUTS" ${ARGN})

    if(NOT TARGET "${target}")
        message(FATAL_ERROR
            "wgfx_target_add_shaders: target '${target}' does not exist")
    endif()
    if(NOT ARG_OUTPUTS)
        return()
    endif()

    set(_shader_target "${target}_shaders")
    add_custom_target(${_shader_target} DEPENDS ${ARG_OUTPUTS})
    add_dependencies("${target}" ${_shader_target})

    wgfx_shaderc_target(_shaderc_target)
    if(_shaderc_target)
        add_dependencies(${_shader_target} ${_shaderc_target})
    endif()
endfunction()
