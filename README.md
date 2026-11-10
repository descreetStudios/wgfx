<div align="center">

# WGFX

### **Wink Graphics** - The Graphics API Foundation

*One graphics API. Every Wink renderer.*

<br>

![C++](https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat&logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-build-28A745?style=flat&logo=cmake&logoColor=white)
[![CI](.github/badges/ci.svg)](https://github.com/descreetStudios/wgfx/actions/workflows/ci.yml)

</div>

## What is WGFX?

**WGFX** (*Wink Graphics*) is the graphics API foundation of the Wink ecosystem. 
It exposes the low-level graphics API surface and leaves specific rendering concepts to the user.
It is built on top of [bgfx](https://github.com/bkaradzic/bgfx) and uses
[wcore](https://github.com/descreetStudios/wcore) as its foundation library.

> All public headers live under `include/wgfx/`, so everything is included with
> the `wgfx/` prefix.

## Getting Started

### Requirements

- **C++20 compiler**: **MSVC 19.29+**, **GCC 13+** or **Clang 17+**
- **CMake** 3.21 or newer (the presets additionally need Ninja)
- **Ninja** (only when using the bundled CMake presets)

Tested on Windows with MSVC.

## Building

```bash
# Configure
cmake -S . -B build

# Compile
cmake --build build --config release
```

Use `--config debug` or `--config reldeb` for the other build types.

## Dependencies

`wcore` and `bgfx` are fetched automatically; no manual checkout is required.

- **wcore** is pinned in `CMakeLists.txt`. To use a different branch or tag:

```bash
cmake --preset release -DWGFX_WCORE_TAG=master
```

To develop against a local wcore checkout:

```bash
cmake --preset release -DFETCHCONTENT_SOURCE_DIR_WCORE=/path/to/wcore
```

- **bgfx** comes from the maintained
  [bgfx.cmake](https://github.com/bkaradzic/bgfx.cmake) port and is linked
  privately. To use a local checkout:

```bash
cmake --preset release -DWGFX_BGFX_CMAKE_DIR=/path/to/bgfx.cmake
```

To build the core library without bgfx:

```bash
cmake --preset release -DWGFX_WITH_BGFX=OFF
```

## Use it in your project

```cpp
#include <wgfx/wgfx.hpp>

#include <wcore/logger.hpp>

int main()
{
    wcore::logger::init();

    wgfx::DeviceDesc desc;
    desc.renderer = wgfx::Renderer::Noop; // Auto picks a real backend.
    desc.width = 1280;
    desc.height = 720;

    auto device = wgfx::Device::create(desc);
    if (!device)
        return 1;

    bool running{ true };
    while (running)
    {
        device->frame();
        // Submit rendering here; set running = false to exit.
    }
}
```

`wgfx::Device` owns the global bgfx context. Only one device may be alive at a
time; `create()` returns `nullptr` if another one is active or initialization
fails. The window itself is owned by the caller and passed through
`DeviceDesc::window`.

Set `DeviceDesc::callback` to receive fatal errors, trace messages, profiler
markers, captures and screen shots. When it is null, bgfx trace output is
forwarded to `wcore::logger`, so initialize the logger first as shown above.
`Device::request_screen_shot()` accepts a swap-chain framebuffer and a file
path; the image is delivered to `Callback::screen_shot`.

## Testing

WGFX uses wcore's tiny, homegrown test framework: **`test.hpp`**. No heavy
external dependencies, just include the header, write your tests, and let
CMake/CTest run them.

```bash
cd build
ctest -C release --output-on-failure
```

Each test file is registered with CTest through CMake, matching the wcore
layout, which keeps tests fast, isolated and easy to add.

## Contributing

Working on a Wink project and need something in the graphics layer?

1. Keep it **bgfx-first**: build on bgfx instead of hiding it behind a new API
   layer.
2. Keep it **small and focused**: one clear responsibility per module.
3. Add a **test** alongside every new feature.
4. Keep the public API **clean**, stable and documented.
5. Keep bgfx out of public headers; it is a private implementation detail.

<br>
<br>

<div align="center">

**Made with ♥ for the Wink ecosystem**

*The project contains monitored AI written code*.

</div>
