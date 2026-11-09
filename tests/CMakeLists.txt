foreach(name test_wgfx test_device test_vertex_layout 
        test_vertex_buffer test_index_buffer test_shader
        test_program test_texture test_uniform test_framebuffer
        test_dynamic_vertex_buffer test_dynamic_index_buffer
        test_indirect_buffer test_occlusion_query test_view test_compute)
    add_executable(${name}
        "${name}.cpp"
        "${WGFX_WCORE_SOURCE_DIR}/src/test.cpp")
    target_link_libraries(${name} PRIVATE wgfx)
    add_test(NAME wgfx.${name} COMMAND ${name})
endforeach()

# --- Shader toolchain --- #
if(WGFX_WITH_BGFX)
    wgfx_shaderc_available(WGFX_SHADERC_AVAILABLE)
    if(WGFX_SHADERC_AVAILABLE)
        set(_shader_source_dir "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

        wgfx_add_shader(
            OUTPUT _test_vs
            SOURCE "${_shader_source_dir}/vs_test.sc"
            STAGE vertex
            VARYINGDEF "${_shader_source_dir}/varying.def.sc")
        wgfx_add_shader(
            OUTPUT _test_fs
            SOURCE "${_shader_source_dir}/fs_test.sc"
            STAGE fragment
            VARYINGDEF "${_shader_source_dir}/varying.def.sc")
        wgfx_add_shader(
            OUTPUT _test_cs
            SOURCE "${_shader_source_dir}/cs_test.sc"
            STAGE compute)

        if(WGFX_SHADER_PLATFORM)
            set(_test_platform "${WGFX_SHADER_PLATFORM}")
        else()
            wgfx_shader_host_platform(_test_platform)
        endif()
        get_filename_component(_test_shader_dir
            "${WGFX_SHADER_OUTPUT_DIR}/${_test_platform}" ABSOLUTE
            BASE_DIR "${CMAKE_CURRENT_BINARY_DIR}")

        add_executable(test_shader_toolchain
            "test_shader_toolchain.cpp"
            "${WGFX_WCORE_SOURCE_DIR}/src/test.cpp")
        target_link_libraries(test_shader_toolchain PRIVATE wgfx)
        target_compile_definitions(test_shader_toolchain PRIVATE
            WGFX_TEST_SHADER_DIR="${_test_shader_dir}")
        wgfx_target_add_shaders(test_shader_toolchain
            OUTPUTS ${_test_vs} ${_test_fs} ${_test_cs})

        add_test(NAME wgfx.test_shader_toolchain COMMAND test_shader_toolchain)
    else()
        message(STATUS
            "wgfx: shaderc not enabled (WGFX_WITH_SHADERC=OFF); "
            "shader toolchain tests not registered")
    endif()
endif()
