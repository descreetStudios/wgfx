#pragma once

#include <wgfx/render_state.hpp>
#include <wgfx/texture.hpp>

namespace wgfx
{
	class DynamicIndexBuffer;
	class DynamicVertexBuffer;
	class FrameBuffer;
	class IndexBuffer;
	class OcclusionQuery;
	class Program;
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

	// A non-owning handle to a bgfx view. Views configure a render target and
	// collect draw submissions for that target. Operations are no-ops when the
	// view belongs to a device that has been shut down.
	class View
	{
	public:
		static constexpr u16 InvalidId{ 0xffff };

		View() = default;

		static View create(u16 id = 0);

		bool valid() const;
		u16 id() const;

		// View configuration.
		bool setRect(
			i16 x,
			i16 y,
			u16 width,
			u16 height,
			float minDepth = 0.0f,
			float maxDepth = 1.0f) const;
		bool setRect(i16 x, i16 y, BackbufferRatio ratio) const;
		bool setScissor(
			u16 x = 0,
			u16 y = 0,
			u16 width = 0,
			u16 height = 0) const;
		bool setClear(
			ClearFlags flags,
			u32 rgba = 0x000000ff,
			float depth = 1.0f,
			u8 stencil = 0) const;
		bool setFrameBuffer(const FrameBuffer* framebuffer = nullptr) const;
		bool setTransform(const void* view, const void* projection) const;
		bool setMode(ViewMode mode) const;
		bool setName(const char* name) const;
		bool resetSettings() const;
		bool touch() const;

		// Draw submission state. State set here applies to the next submit.
		bool setState(const RenderState& state) const;
		bool setState(RenderStateFlags flags, u32 rgba = 0) const;

		bool setVertexBuffer(u8 stream, const VertexBuffer& buffer) const;
		bool setVertexBuffer(
			u8 stream,
			const VertexBuffer& buffer,
			u32 startVertex,
			u32 numVertices) const;
		bool setVertexBuffer(u8 stream, const DynamicVertexBuffer& buffer) const;
		bool setVertexBuffer(
			u8 stream,
			const DynamicVertexBuffer& buffer,
			u32 startVertex,
			u32 numVertices) const;

		bool setIndexBuffer(const IndexBuffer& buffer) const;
		bool setIndexBuffer(
			const IndexBuffer& buffer,
			u32 firstIndex,
			u32 numIndices) const;
		bool setIndexBuffer(const DynamicIndexBuffer& buffer) const;
		bool setIndexBuffer(
			const DynamicIndexBuffer& buffer,
			u32 firstIndex,
			u32 numIndices) const;

		// Binds `texture` to `stage` using the texture's sampler settings.
		bool setTexture(
			u8 stage,
			const Uniform& sampler,
			const Texture& texture) const;
		bool setTexture(
			u8 stage,
			const Uniform& sampler,
			const Texture& texture,
			SamplerFlags flags) const;

		bool submit(
			const Program& program,
			u32 depth = 0,
			DiscardFlags flags = DiscardFlags::All) const;
		bool submit(
			const Program& program,
			const OcclusionQuery& query,
			u32 depth = 0,
			DiscardFlags flags = DiscardFlags::All) const;
		bool discard(DiscardFlags flags = DiscardFlags::All) const;

	private:
		u16 mId{ InvalidId };
		u32 mGeneration{ 0 };
	};
} // namespace wgfx
