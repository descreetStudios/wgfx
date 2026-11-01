foreach(name test_wgfx test_device)
    add_executable(${name}
        "${name}.cpp"
        "${WGFX_WCORE_SOURCE_DIR}/src/test.cpp")
    target_link_libraries(${name} PRIVATE wgfx)
    add_test(NAME wgfx.${name} COMMAND ${name})
endforeach()
