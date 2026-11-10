#pragma once

#include <wgfx/access.hpp>
#include <wgfx/render_state.hpp>
#include <wgfx/texture.hpp>

#include <wcore/types.hpp>

namespace wgfx
{
	class DynamicIndexBuffer;
	class DynamicVertexBuffer;
	class IndexBuffer;
	class IndirectBuffer;
	class InstanceDataBuffer;
	class OcclusionQuery;
	class Program;
	class TransientIndexBuffer;
	class TransientVertexBuffer;
	class Uniform;
	class VertexBuffer;
	class View;

	class Encoder
	{
	public:
		static constexpr u32 WholeBuffer{ 0xffffffffu };

		Encoder() = default;
		~Encoder();

		Encoder(const Encoder&) = delete;
		Encoder& operator=(const Encoder&) = delete;

		Encoder(Encoder&& other) noexcept;
		Encoder& operator=(Encoder&& other) noexcept;

		static Encoder create(bool forceNewEncoder = false);

		bool valid() const;
		void reset();

		bool set_marker(const char* name, i32 length = -1) const;
		bool set_condition(const OcclusionQuery& query, bool visible) const;

		bool set_state(const RenderState& state) const;
		bool set_state(RenderStateFlags flags, u32 rgba = 0) const;
		bool set_vertex_count(u32 count) const;
		bool set_model_transform(const void* matrix, u16 count = 1) const;
		bool set_draw_scissor(u16 x, u16 y, u16 width, u16 height) const;
		bool set_depth_control(
			i32 constant = 0,
			float slopeScale = 0.0f,
			float clamp = 0.0f,
			bool depthClamp = false) const;
		bool set_sample_mask(u32 mask = 0xffffffffu) const;
		bool set_stencil(u32 front, u32 back = StencilNone) const;

		bool set_vertex_buffer(u8 stream, const VertexBuffer& buffer) const;
		bool set_vertex_buffer(u8 stream, const VertexBuffer& buffer,
			u32 startVertex, u32 numVertices) const;
		bool set_vertex_buffer(u8 stream, const DynamicVertexBuffer& buffer) const;
		bool set_vertex_buffer(u8 stream, const DynamicVertexBuffer& buffer,
			u32 startVertex, u32 numVertices) const;
		bool set_vertex_buffer(u8 stream, const TransientVertexBuffer& buffer) const;
		bool set_vertex_buffer(u8 stream, const TransientVertexBuffer& buffer,
			u32 startVertex, u32 numVertices) const;

		bool set_index_buffer(const IndexBuffer& buffer) const;
		bool set_index_buffer(const IndexBuffer& buffer,
			u32 firstIndex, u32 numIndices) const;
		bool set_index_buffer(const DynamicIndexBuffer& buffer) const;
		bool set_index_buffer(const DynamicIndexBuffer& buffer,
			u32 firstIndex, u32 numIndices) const;
		bool set_index_buffer(const TransientIndexBuffer& buffer) const;
		bool set_index_buffer(const TransientIndexBuffer& buffer,
			u32 firstIndex, u32 numIndices) const;

		bool set_instance_count(u32 count) const;
		bool set_instance_data_buffer(const VertexBuffer& buffer,
			u32 startVertex, u32 numInstances) const;
		bool set_instance_data_buffer(const DynamicVertexBuffer& buffer,
			u32 startVertex, u32 numInstances) const;
		bool set_instance_data_buffer(const InstanceDataBuffer& buffer,
			u32 start = 0, u32 num = WholeBuffer) const;

		bool set_texture(u8 stage,
			const Uniform& sampler,
			const Texture& texture) const;
		bool set_texture(u8 stage,
			const Uniform& sampler,
			const Texture& texture,
			SamplerFlags flags) const;

		bool set_image(u8 stage,
			const Texture& texture,
			u8 mip,
			Access access,
			TextureFormat format = TextureFormat::Count) const;
		bool set_image(u8 stage,
			const Texture& texture,
			u16 firstLayer,
			u16 numLayers,
			u8 mip,
			Access access,
			TextureFormat format = TextureFormat::Count) const;

		bool set_buffer(u8 stage,
			const VertexBuffer& buffer,
			Access access,
			u32 offset = 0,
			u32 size = WholeBuffer) const;
		bool set_buffer(u8 stage,
			const IndexBuffer& buffer,
			Access access,
			u32 offset = 0,
			u32 size = WholeBuffer) const;
		bool set_buffer(u8 stage,
			const DynamicVertexBuffer& buffer,
			Access access,
			u32 offset = 0,
			u32 size = WholeBuffer) const;
		bool set_buffer(u8 stage,
			const DynamicIndexBuffer& buffer,
			Access access,
			u32 offset = 0,
			u32 size = WholeBuffer) const;
		bool set_buffer(u8 stage,
			const IndirectBuffer& buffer,
			Access access) const;

		bool touch(const View& view) const;

		bool dispatch(const View& view,
			const Program& program,
			u32 numX = 1,
			u32 numY = 1,
			u32 numZ = 1,
			DiscardFlags flags = DiscardFlags::All) const;
		bool dispatch(const View& view,
			const Program& program,
			const IndirectBuffer& indirect,
			u32 start = 0,
			u32 num = 1,
			DiscardFlags flags = DiscardFlags::All) const;

		bool submit(const View& view,
			const Program& program,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool submit(const View& view,
			const Program& program,
			const OcclusionQuery& query,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool submit(const View& view,
			const Program& program,
			const IndirectBuffer& indirect,
			u32 start = 0, u32 num = 1,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool submit(const View& view,
			const Program& program,
			const IndirectBuffer& indirect,
			u32 start, const IndexBuffer& numBuffer,
			u32 numIndex = 0, u32 numMax = 0xffffffffu,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;

		bool discard(DiscardFlags flags = DiscardFlags::All) const;

		bool blit(const View& view,
			const TextureRegion& dst,
			const TextureRegion& src) const;

	private:
		void* mEncoder{ nullptr };
		u32 mGeneration{ 0 };
	};
}
