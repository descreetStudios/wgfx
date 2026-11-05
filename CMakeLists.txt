cmake_minimum_required(VERSION 3.21)
project(wgfx VERSION 0.1.0 LANGUAGES CXX)

add_library(wgfx STATIC
    "src/version.cpp"
    "src/backend.cpp"
    "src/device.cpp"
    "src/vertex_buffer.cpp"
    "src/dynamic_vertex_buffer.cpp"
    "src/vertex_layout.cpp"
    "src/index_buffer.cpp"
    "src/dynamic_index_buffer.cpp"
    "src/shader.cpp"
    "src/program.cpp"
    "src/texture.cpp"
    "src/uniform.cpp"
    "src/framebuffer.cpp"
    "src/occlusion_query.cpp")
add_library(wgfx::wgfx ALIAS wgfx)

# --- Definitions --- #
target_compile_features(wgfx
PUBLIC
    cxx_std_20)
target_compile_definitions(wgfx
PUBLIC
    $<$<CONFIG:Debug>:WGFX_DEBUG=1>)

# --- FetchContent --- #
include(FetchContent)

# --- wcore --- #
set(WGFX_WCORE_REPOSITORY "https://github.com/descreetStudios/wcore.git" CACHE STRING
    "Git repository for wcore")
set(WGFX_WCORE_TAG "master" CACHE STRING
    "Git tag or commit for wcore")

FetchContent_Declare(
    wcore
    GIT_REPOSITORY "${WGFX_WCORE_REPOSITORY}"
    GIT_TAG        "${WGFX_WCORE_TAG}"
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(wcore)
FetchContent_GetProperties(wcore SOURCE_DIR WGFX_WCORE_SOURCE_DIR)

# --- bgfx --- #
option(WGFX_WITH_BGFX "Build the bgfx rendering backend" ON)
if(WGFX_WITH_BGFX)
    set(WGFX_BGFX_CMAKE_DIR "" CACHE PATH
        "Path to a local bgfx.cmake checkout (empty: fetch from GitHub)")

    if(WGFX_BGFX_CMAKE_DIR)
        add_subdirectory("${WGFX_BGFX_CMAKE_DIR}"
            "${CMAKE_BINARY_DIR}/_deps/bgfx-cmake-build")
    else()
        set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
        set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BGFX_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
        set(BGFX_CUSTOM_TARGETS OFF CACHE BOOL "" FORCE)

        FetchContent_Declare(
            bgfx_cmake
            GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake.git
            GIT_TAG        v1.167.9605-606
            GIT_SHALLOW    TRUE)
        FetchContent_MakeAvailable(bgfx_cmake)
    endif()

    target_link_libraries(wgfx PRIVATE bgfx)
    target_compile_definitions(wgfx PUBLIC WGFX_BACKEND_BGFX=1)
endif()

# --- Targets --- #
target_include_directories(wgfx
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(wgfx
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(wgfx PUBLIC wcore::wcore)

# --- Tests --- #
option(WGFX_BUILD_TESTS "Build wgfx tests" ${PROJECT_IS_TOP_LEVEL})
if(WGFX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
