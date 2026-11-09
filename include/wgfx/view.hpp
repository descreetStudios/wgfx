#pragma once

#include <wgfx/access.hpp>
#include <wgfx/render_state.hpp>
#include <wgfx/texture.hpp>

namespace wgfx
{
	class DynamicIndexBuffer;
	class DynamicVertexBuffer;
	class FrameBuffer;
	class IndexBuffer;
	class IndirectBuffer;
	class InstanceDataBuffer;
	class OcclusionQuery;
	class Program;
	class TransientIndexBuffer;
	class TransientVertexBuffer;
	class Uniform;
	class VertexBuffer;

	enum class ViewMode : u8
	{
		Default,
		Sequential,
		DepthAscending,
		DepthDescending,

		Count,
	};

	// Bit flags matching bgfx's BGFX_CLEAR_* values.
	enum class ClearFlags : u16
	{
		None = 0x0000,
		Color = 0x0001,
		Depth = 0x0002,
		Stencil = 0x0004,

		DiscardColor0 = 0x0008,
		DiscardColor1 = 0x0010,
		DiscardColor2 = 0x0020,
		DiscardColor3 = 0x0040,
		DiscardColor4 = 0x0080,
		DiscardColor5 = 0x0100,
		DiscardColor6 = 0x0200,
		DiscardColor7 = 0x0400,
		DiscardDepth = 0x0800,
		DiscardStencil = 0x1000,

		ColorDepth = 0x0003,
		ColorDepthStencil = 0x0007,
		DiscardColorMask = 0x07f8,
		DiscardMask = 0x1ff8,
	};

	constexpr ClearFlags operator|(ClearFlags lhs, ClearFlags rhs) noexcept
	{
		return static_cast<ClearFlags>(
			static_cast<u16>(lhs) | static_cast<u16>(rhs));
	}

	constexpr ClearFlags operator&(ClearFlags lhs, ClearFlags rhs) noexcept
	{
		return static_cast<ClearFlags>(
			static_cast<u16>(lhs) & static_cast<u16>(rhs));
	}

	constexpr ClearFlags& operator|=(ClearFlags& lhs, ClearFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	class View
	{
	public:
		static constexpr u16 InvalidId{ 0xffff };
		static constexpr u32 WholeBuffer{ 0xffffffffu };

		View() = default;

		static View create(u16 id = 0);

		bool valid() const;
		u16 id() const;

		bool set_rect(i16 x, i16 y,
			u16 width, u16 height,
			float minDepth = 0.0f,
			float maxDepth = 1.0f) const;
		bool set_rect(i16 x, i16 y, BackbufferRatio ratio) const;
		bool set_scissor(u16 x = 0, u16 y = 0,
			u16 width = 0, u16 height = 0) const;
		bool set_clear(ClearFlags flags,
			u32 rgba = 0x000000ff,
			float depth = 1.0f,
			u8 stencil = 0) const;
		bool set_framebuffer(const FrameBuffer* framebuffer = nullptr) const;
		bool set_transform(const void* view, const void* projection) const;
		bool set_mode(ViewMode mode) const;
		bool set_name(const char* name) const;
		bool reset_settings() const;
		bool touch() const;

		bool set_state(const RenderState& state) const;
		bool set_state(RenderStateFlags flags, u32 rgba = 0) const;

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

		bool dispatch(const Program& program,
			u32 numX = 1,
			u32 numY = 1,
			u32 numZ = 1,
			DiscardFlags flags = DiscardFlags::All) const;
		bool dispatch(const Program& program,
			const IndirectBuffer& indirect,
			u32 start = 0,
			u32 num = 1,
			DiscardFlags flags = DiscardFlags::All) const;

		bool submit(const Program& program,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool submit(const Program& program,
			const OcclusionQuery& query,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool submit(const Program& program,
			const IndirectBuffer& indirect,
			u32 start = 0, u32 num = 1,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool submit(const Program& program,
			const IndirectBuffer& indirect,
			u32 start, const IndexBuffer& numBuffer,
			u32 numIndex = 0, u32 numMax = 0xffffffffu,
			u32 depth = 0, DiscardFlags flags = DiscardFlags::All) const;
		bool discard(DiscardFlags flags = DiscardFlags::All) const;

	private:
		u16 mId{ InvalidId };
		u32 mGeneration{ 0 };
	};
}