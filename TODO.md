# TODO

## Core
- [x] CMake skeleton
- [x] wcore integration
- [x] bgfx integration
- [x] Device/context wrapper

## Resources
- [x] VertexLayout
- [x] VertexBuffer
- [x] IndexBuffer
- [x] Shader
- [x] Program
- [x] Texture
- [x] Uniform
- [x] FrameBuffer
- [x] DynamicVertexBuffer
- [x] DynamicIndexBuffer
- [x] IndirectBuffer
- [x] OcclusionQuery

## Rendering
- [x] Views, render state and draw submission
- [x] Shader pipeline and toolchain
- [x] Compute dispatch, images and indirect dispatch
- [x] Resize/reset and multi-window frame buffers
- [x] Instancing and indirect draws
- [x] Transient buffers
- [x] Advanced view/draw state
- [x] Diagnostics and debugging


### Multithreaded submission
- [x] Add `wgfx::Encoder` (`include/wgfx/encoder.hpp`) wrapping `bgfx::Encoder`
      with the draw-side `View` API: state, transforms, vertex/index/instance
      buffers, textures, images, storage buffers, `submit`, `dispatch`,
      `discard`, `blit` and markers.
- [x] Define encoder begin/end semantics and document that resources are still
      created and destroyed on the API thread.
- [x] Add `DeviceDesc::maxEncoders` (mapped to bgfx init limits) and expose
      `DeviceCaps::maxEncoders`.
- [x] Add `Device::render_frame()` for applications that drive a dedicated
      render thread.
- [x] Test parallel encoder submission to different views on the Noop backend
      and verify encoder stats.

### Callback and capture
- [x] Add a `wgfx::Callback` interface for fatal errors, trace messages,
      profiler markers, frame capture and screen shots.
- [x] Add `DeviceDesc::callback` and provide a default adapter that forwards
      bgfx trace output to `wcore::logger`.
- [x] Implement `Device::request_screen_shot(const FrameBuffer&, const char*)`.
- [x] Add a typed `FrameFlags` enum and a `Device::frame(FrameFlags)` overload
      for `BGFX_FRAME_DEBUG_CAPTURE`.
- [x] Test callback forwarding and screen-shot request validation.

### Device and swap-chain configuration
- [ ] Let `DeviceDesc` configure the main swap chain through `SwapChainDesc`
      (color/depth format, MSAA, sRGB, HDR10, HiDPI, back-buffer count and
      frame latency).
- [ ] Expose every relevant reset/swap-chain flag instead of deriving reset
      flags from vsync alone.
- [ ] Add platform interop data (D3D/Vulkan context and queue) to
      `NativeWindow`/`DeviceDesc`.
- [ ] Add vendor/device selection, renderer fallback and init limits.
- [ ] Test configuration round-trips on the Noop backend.

### Capabilities and queries
- [ ] Mirror all `bgfx::Caps::Limits` in `DeviceCaps` (draw calls, blits,
      attachments, encoders, uniform cache sizes, blit alignment, etc.).
- [ ] Add typed capability flags and
      `DeviceCaps::is_format_supported(format, type, usage)` backed by bgfx's
      format table.
- [ ] Expose the GPU list and `Renderer::supported()` /
      `Device::supported_renderers()`.

### Blit, clear and introspection
- [ ] Add a `BufferRegion` type that can reference any WGFX buffer wrapper.
- [ ] Add `View::blit` overloads for texture-to-texture, buffer-to-texture,
      texture-to-buffer and buffer-to-buffer copies.
- [ ] Add `Texture::clear(mip, numMips, layer, numLayers)`.
- [ ] Add `TextureInfo` with `Texture::info()` and validation helpers such as
      `Texture::is_valid(desc)` and `FrameBuffer::is_valid(desc)`.
- [ ] Add `FrameBuffer::attachment_texture(u8 index)` so MRT attachments can be
      read back.
- [ ] Add `Texture::direct_access()` when the backend reports direct texture
      access support.
- [ ] Treat zero-sized `TextureRegion` fields as "the rest of the mip" to match
      bgfx semantics consistently.

### Palette and per-attachment clears
- [ ] Add `Device::set_palette_color(index, rgba)` overloads.
- [ ] Add a `View::set_clear` overload that takes a palette index per color
      attachment.
- [ ] Test palette-backed clears with a multi-attachment frame buffer.

### Geometry utilities
- [ ] Add `vertex_pack`, `vertex_unpack` and `vertex_convert` helpers backed by
      `VertexLayout`.
- [ ] Add `TopologyConvert` / `TopologySort` enums and
      `topology_convert` / `topology_sort_tri_list`.
- [ ] Expose `calc_texture_size(...)` and test it against known bgfx values.

### Shader toolchain
- [ ] Add `PLATFORMS`/multi-profile output to `wgfx_add_shader` so one build
      can produce D3D, Vulkan, Metal and other shader binaries.
- [ ] Add shader embedding (bin2c-style) so renderers can avoid loose `.bin`
      files.
- [ ] Add a runtime helper that selects the shader path for the active
      renderer.
- [ ] Run `test_shader_toolchain` in CI with `WGFX_WITH_SHADERC=ON`.

### Packaging and docs
- [ ] Add `install(TARGETS wgfx EXPORT wgfxTargets)` plus public headers,
      `wgfxConfig.cmake`, `wgfxConfigVersion.cmake` and a `find_package(wgfx)`
      smoke test.
- [ ] Install `WgfxShaders.cmake` and document the installed shader toolchain.
- [ ] Add a minimal windowed triangle example and a headless compute/texture
      example under a `WGFX_BUILD_EXAMPLES` option.
- [ ] Document threading rules, handle generation/lifetime rules, coordinate
      and depth conventions, and the shader workflow.

### Quality and CI
- [ ] Add a Clang build/test job and an ASan/UBSan Linux job.
- [ ] Add a header self-containment test that compiles every public
      `wgfx/*.hpp` on its own.
- [ ] Add missing unit tests for `RenderState` helper constants and
      `DeviceCaps`.
- [ ] Add debug assertions (`wcore/assert.hpp`) for invalid handle, device and
      thread usage.
