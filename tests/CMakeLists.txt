foreach(name test_wgfx test_device test_vertex_layout 
        test_vertex_buffer test_index_buffer test_shader
        test_program test_texture test_uniform test_framebuffer
        test_dynamic_vertex_buffer test_dynamic_index_buffer
        test_occlusion_query)
    add_executable(${name}
        "${name}.cpp"
        "${WGFX_WCORE_SOURCE_DIR}/src/test.cpp")
    target_link_libraries(${name} PRIVATE wgfx)
    add_test(NAME wgfx.${name} COMMAND ${name})
endforeach()
