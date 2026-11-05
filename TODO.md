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
	- [ ] Uniform introspection via `getShaderUniforms`
- [x] Program
- [x] Texture
	- [ ] `updateTexture*`, `blit`, `readTexture`
	- [ ] Loading from image containers via `createTexture(Memory*)`
- [x] Uniform
- [x] FrameBuffer
	- [x] Ratio-based creation
	- [ ] Swap-chain/window creation and resize
- [x] DynamicVertexBuffer
- [x] DynamicIndexBuffer
- [ ] IndirectBuffer
- [x] OcclusionQuery

## Rendering
- [ ] Shader pipeline and toolchain (shaderc via bgfx.cmake)
- [ ] Views, render state and draw submission
- [ ] Compute dispatch, images and indirect dispatch
- [ ] Resize/reset and multi-window frame buffers
