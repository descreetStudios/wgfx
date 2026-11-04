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
- [x] Texture (2D and texture arrays)
	- [ ] 3D textures
	- [ ] Cube and cube-array textures
	- [ ] `updateTexture*`, `blit`, `readTexture`
	- [ ] Loading from image containers via `createTexture(Memory*)`
- [x] Uniform
	- [ ] Creation frequency (`UniformFreq::View` / `Frame`)
- [x] FrameBuffer
	- [ ] Accessor for owned textures for sampling
	- [ ] Ratio-based creation
	- [ ] Swap-chain/window creation and resize
- [ ] DynamicVertexBuffer
- [ ] DynamicIndexBuffer
- [ ] IndirectBuffer
- [ ] OcclusionQuery

## Rendering
- [ ] Shader pipeline and toolchain (shaderc via bgfx.cmake)
- [ ] Views, render state and draw submission
- [ ] Compute dispatch, images and indirect dispatch
- [ ] Resize/reset and multi-window frame buffers
