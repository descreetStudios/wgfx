#include <wgfx/view.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "dynamic_index_buffer_internal.hpp"
#include "dynamic_vertex_buffer_internal.hpp"
#include "framebuffer_internal.hpp"
#include "index_buffer_internal.hpp"
#include "occlusion_query_internal.hpp"
#include "program_internal.hpp"
#include "texture_internal.hpp"
#include "uniform_internal.hpp"
#include "vertex_buffer_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		constexpr u8 MaxVertexStreams{ 4 };
		constexpr u8 MaxTextureStages{ 16 };
		constexpr u16 ClearFlagsMask{ 0x1fff };

		bool valid_stream(u8 stream)
		{
			return stream < MaxVertexStreams;
		}

		bool valid_stage(u8 stage)
		{
			return stage < MaxTextureStages;
		}

		bgfx::ViewMode::Enum to_bgfx(ViewMode mode)
		{
			return static_cast<bgfx::ViewMode::Enum>(mode);
		}

		bgfx::BackbufferRatio::Enum to_bgfx(BackbufferRatio ratio)
		{
			return ratio == BackbufferRatio::Explicit
				? bgfx::BackbufferRatio::Count
				: static_cast<bgfx::BackbufferRatio::Enum>(ratio);
		}

#define WGFX_STATE_MIRROR(name, bgfxName) \
		static_assert(static_cast<u64>(RenderStateFlags::name) == (bgfxName), \
			"RenderStateFlags::" #name " must mirror " #bgfxName)

		WGFX_STATE_MIRROR(None, BGFX_STATE_NONE);
		WGFX_STATE_MIRROR(WriteR, BGFX_STATE_WRITE_R);
		WGFX_STATE_MIRROR(WriteG, BGFX_STATE_WRITE_G);
		WGFX_STATE_MIRROR(WriteB, BGFX_STATE_WRITE_B);
		WGFX_STATE_MIRROR(WriteA, BGFX_STATE_WRITE_A);
		WGFX_STATE_MIRROR(WriteZ, BGFX_STATE_WRITE_Z);
		WGFX_STATE_MIRROR(WriteRgb, BGFX_STATE_WRITE_RGB);
		WGFX_STATE_MIRROR(WriteMask, BGFX_STATE_WRITE_MASK);
		WGFX_STATE_MIRROR(DepthTestLess, BGFX_STATE_DEPTH_TEST_LESS);
		WGFX_STATE_MIRROR(DepthTestLequal, BGFX_STATE_DEPTH_TEST_LEQUAL);
		WGFX_STATE_MIRROR(DepthTestEqual, BGFX_STATE_DEPTH_TEST_EQUAL);
		WGFX_STATE_MIRROR(DepthTestGequal, BGFX_STATE_DEPTH_TEST_GEQUAL);
		WGFX_STATE_MIRROR(DepthTestGreater, BGFX_STATE_DEPTH_TEST_GREATER);
		WGFX_STATE_MIRROR(DepthTestNotequal, BGFX_STATE_DEPTH_TEST_NOTEQUAL);
		WGFX_STATE_MIRROR(DepthTestNever, BGFX_STATE_DEPTH_TEST_NEVER);
		WGFX_STATE_MIRROR(DepthTestAlways, BGFX_STATE_DEPTH_TEST_ALWAYS);
		WGFX_STATE_MIRROR(BlendZero, BGFX_STATE_BLEND_ZERO);
		WGFX_STATE_MIRROR(BlendOne, BGFX_STATE_BLEND_ONE);
		WGFX_STATE_MIRROR(BlendSrcColor, BGFX_STATE_BLEND_SRC_COLOR);
		WGFX_STATE_MIRROR(BlendInvSrcColor, BGFX_STATE_BLEND_INV_SRC_COLOR);
		WGFX_STATE_MIRROR(BlendSrcAlpha, BGFX_STATE_BLEND_SRC_ALPHA);
		WGFX_STATE_MIRROR(BlendInvSrcAlpha, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		WGFX_STATE_MIRROR(BlendDstAlpha, BGFX_STATE_BLEND_DST_ALPHA);
		WGFX_STATE_MIRROR(BlendInvDstAlpha, BGFX_STATE_BLEND_INV_DST_ALPHA);
		WGFX_STATE_MIRROR(BlendDstColor, BGFX_STATE_BLEND_DST_COLOR);
		WGFX_STATE_MIRROR(BlendInvDstColor, BGFX_STATE_BLEND_INV_DST_COLOR);
		WGFX_STATE_MIRROR(BlendSrcAlphaSat, BGFX_STATE_BLEND_SRC_ALPHA_SAT);
		WGFX_STATE_MIRROR(BlendFactor, BGFX_STATE_BLEND_FACTOR);
		WGFX_STATE_MIRROR(BlendInvFactor, BGFX_STATE_BLEND_INV_FACTOR);
		WGFX_STATE_MIRROR(BlendEquationAdd, BGFX_STATE_BLEND_EQUATION_ADD);
		WGFX_STATE_MIRROR(BlendEquationSub, BGFX_STATE_BLEND_EQUATION_SUB);
		WGFX_STATE_MIRROR(BlendEquationRevSub, BGFX_STATE_BLEND_EQUATION_REVSUB);
		WGFX_STATE_MIRROR(BlendEquationMin, BGFX_STATE_BLEND_EQUATION_MIN);
		WGFX_STATE_MIRROR(BlendEquationMax, BGFX_STATE_BLEND_EQUATION_MAX);
		WGFX_STATE_MIRROR(CullCw, BGFX_STATE_CULL_CW);
		WGFX_STATE_MIRROR(CullCcw, BGFX_STATE_CULL_CCW);
		WGFX_STATE_MIRROR(PrimitiveTriStrip, BGFX_STATE_PT_TRISTRIP);
		WGFX_STATE_MIRROR(PrimitiveLines, BGFX_STATE_PT_LINES);
		WGFX_STATE_MIRROR(PrimitiveLineStrip, BGFX_STATE_PT_LINESTRIP);
		WGFX_STATE_MIRROR(PrimitivePoints, BGFX_STATE_PT_POINTS);
		WGFX_STATE_MIRROR(Msaa, BGFX_STATE_MSAA);
		WGFX_STATE_MIRROR(LineAa, BGFX_STATE_LINEAA);
		WGFX_STATE_MIRROR(ConservativeRaster, BGFX_STATE_CONSERVATIVE_RASTER);
		WGFX_STATE_MIRROR(FrontCcw, BGFX_STATE_FRONT_CCW);
		WGFX_STATE_MIRROR(BlendIndependent, BGFX_STATE_BLEND_INDEPENDENT);
		WGFX_STATE_MIRROR(BlendAlphaToCoverage, BGFX_STATE_BLEND_ALPHA_TO_COVERAGE);

#undef WGFX_STATE_MIRROR

#define WGFX_BLEND_FACTOR_MIRROR(name, bgfxName) \
		static_assert(static_cast<u64>(BlendFactor::name) == (bgfxName), \
			"BlendFactor::" #name " must mirror " #bgfxName)

		WGFX_BLEND_FACTOR_MIRROR(Zero, BGFX_STATE_BLEND_ZERO);
		WGFX_BLEND_FACTOR_MIRROR(One, BGFX_STATE_BLEND_ONE);
		WGFX_BLEND_FACTOR_MIRROR(SrcColor, BGFX_STATE_BLEND_SRC_COLOR);
		WGFX_BLEND_FACTOR_MIRROR(InvSrcColor, BGFX_STATE_BLEND_INV_SRC_COLOR);
		WGFX_BLEND_FACTOR_MIRROR(SrcAlpha, BGFX_STATE_BLEND_SRC_ALPHA);
		WGFX_BLEND_FACTOR_MIRROR(InvSrcAlpha, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		WGFX_BLEND_FACTOR_MIRROR(DstAlpha, BGFX_STATE_BLEND_DST_ALPHA);
		WGFX_BLEND_FACTOR_MIRROR(InvDstAlpha, BGFX_STATE_BLEND_INV_DST_ALPHA);
		WGFX_BLEND_FACTOR_MIRROR(DstColor, BGFX_STATE_BLEND_DST_COLOR);
		WGFX_BLEND_FACTOR_MIRROR(InvDstColor, BGFX_STATE_BLEND_INV_DST_COLOR);
		WGFX_BLEND_FACTOR_MIRROR(SrcAlphaSat, BGFX_STATE_BLEND_SRC_ALPHA_SAT);
		WGFX_BLEND_FACTOR_MIRROR(Factor, BGFX_STATE_BLEND_FACTOR);
		WGFX_BLEND_FACTOR_MIRROR(InvFactor, BGFX_STATE_BLEND_INV_FACTOR);

#undef WGFX_BLEND_FACTOR_MIRROR

#define WGFX_BLEND_EQUATION_MIRROR(name, bgfxName) \
		static_assert(static_cast<u64>(BlendEquation::name) == (bgfxName), \
			"BlendEquation::" #name " must mirror " #bgfxName)

		WGFX_BLEND_EQUATION_MIRROR(Add, BGFX_STATE_BLEND_EQUATION_ADD);
		WGFX_BLEND_EQUATION_MIRROR(Sub, BGFX_STATE_BLEND_EQUATION_SUB);
		WGFX_BLEND_EQUATION_MIRROR(RevSub, BGFX_STATE_BLEND_EQUATION_REVSUB);
		WGFX_BLEND_EQUATION_MIRROR(Min, BGFX_STATE_BLEND_EQUATION_MIN);
		WGFX_BLEND_EQUATION_MIRROR(Max, BGFX_STATE_BLEND_EQUATION_MAX);

#undef WGFX_BLEND_EQUATION_MIRROR

#define WGFX_SAMPLER_MIRROR(name, bgfxName) \
		static_assert(static_cast<u32>(SamplerFlags::name) == (bgfxName), \
			"SamplerFlags::" #name " must mirror " #bgfxName)

		WGFX_SAMPLER_MIRROR(None, BGFX_SAMPLER_NONE);
		WGFX_SAMPLER_MIRROR(UMirror, BGFX_SAMPLER_U_MIRROR);
		WGFX_SAMPLER_MIRROR(UClamp, BGFX_SAMPLER_U_CLAMP);
		WGFX_SAMPLER_MIRROR(UBorder, BGFX_SAMPLER_U_BORDER);
		WGFX_SAMPLER_MIRROR(VMirror, BGFX_SAMPLER_V_MIRROR);
		WGFX_SAMPLER_MIRROR(VClamp, BGFX_SAMPLER_V_CLAMP);
		WGFX_SAMPLER_MIRROR(VBorder, BGFX_SAMPLER_V_BORDER);
		WGFX_SAMPLER_MIRROR(WMirror, BGFX_SAMPLER_W_MIRROR);
		WGFX_SAMPLER_MIRROR(WClamp, BGFX_SAMPLER_W_CLAMP);
		WGFX_SAMPLER_MIRROR(WBorder, BGFX_SAMPLER_W_BORDER);
		WGFX_SAMPLER_MIRROR(MinPoint, BGFX_SAMPLER_MIN_POINT);
		WGFX_SAMPLER_MIRROR(MinAnisotropic, BGFX_SAMPLER_MIN_ANISOTROPIC);
		WGFX_SAMPLER_MIRROR(MagPoint, BGFX_SAMPLER_MAG_POINT);
		WGFX_SAMPLER_MIRROR(MagAnisotropic, BGFX_SAMPLER_MAG_ANISOTROPIC);
		WGFX_SAMPLER_MIRROR(MipPoint, BGFX_SAMPLER_MIP_POINT);
		WGFX_SAMPLER_MIRROR(CompareLess, BGFX_SAMPLER_COMPARE_LESS);
		WGFX_SAMPLER_MIRROR(CompareLequal, BGFX_SAMPLER_COMPARE_LEQUAL);
		WGFX_SAMPLER_MIRROR(CompareEqual, BGFX_SAMPLER_COMPARE_EQUAL);
		WGFX_SAMPLER_MIRROR(CompareGequal, BGFX_SAMPLER_COMPARE_GEQUAL);
		WGFX_SAMPLER_MIRROR(CompareGreater, BGFX_SAMPLER_COMPARE_GREATER);
		WGFX_SAMPLER_MIRROR(CompareNotequal, BGFX_SAMPLER_COMPARE_NOTEQUAL);
		WGFX_SAMPLER_MIRROR(CompareNever, BGFX_SAMPLER_COMPARE_NEVER);
		WGFX_SAMPLER_MIRROR(CompareAlways, BGFX_SAMPLER_COMPARE_ALWAYS);
		WGFX_SAMPLER_MIRROR(SampleStencil, BGFX_SAMPLER_SAMPLE_STENCIL);
		WGFX_SAMPLER_MIRROR(Srgb, BGFX_SAMPLER_SRGB);
		WGFX_SAMPLER_MIRROR(Point, BGFX_SAMPLER_POINT);
		WGFX_SAMPLER_MIRROR(UvwMirror, BGFX_SAMPLER_UVW_MIRROR);
		WGFX_SAMPLER_MIRROR(UvwClamp, BGFX_SAMPLER_UVW_CLAMP);
		WGFX_SAMPLER_MIRROR(UvwBorder, BGFX_SAMPLER_UVW_BORDER);

#undef WGFX_SAMPLER_MIRROR

#define WGFX_DISCARD_MIRROR(name, bgfxName) \
		static_assert(static_cast<u8>(DiscardFlags::name) == (bgfxName), \
			"DiscardFlags::" #name " must mirror " #bgfxName)

		WGFX_DISCARD_MIRROR(None, BGFX_DISCARD_NONE);
		WGFX_DISCARD_MIRROR(Bindings, BGFX_DISCARD_BINDINGS);
		WGFX_DISCARD_MIRROR(IndexBuffer, BGFX_DISCARD_INDEX_BUFFER);
		WGFX_DISCARD_MIRROR(InstanceData, BGFX_DISCARD_INSTANCE_DATA);
		WGFX_DISCARD_MIRROR(State, BGFX_DISCARD_STATE);
		WGFX_DISCARD_MIRROR(Transform, BGFX_DISCARD_TRANSFORM);
		WGFX_DISCARD_MIRROR(VertexStreams, BGFX_DISCARD_VERTEX_STREAMS);
		WGFX_DISCARD_MIRROR(All, BGFX_DISCARD_ALL);

#undef WGFX_DISCARD_MIRROR

#define WGFX_CLEAR_MIRROR(name, bgfxName) \
		static_assert(static_cast<u16>(ClearFlags::name) == (bgfxName), \
			"ClearFlags::" #name " must mirror " #bgfxName)

		WGFX_CLEAR_MIRROR(None, BGFX_CLEAR_NONE);
		WGFX_CLEAR_MIRROR(Color, BGFX_CLEAR_COLOR);
		WGFX_CLEAR_MIRROR(Depth, BGFX_CLEAR_DEPTH);
		WGFX_CLEAR_MIRROR(Stencil, BGFX_CLEAR_STENCIL);
		WGFX_CLEAR_MIRROR(DiscardColor0, BGFX_CLEAR_DISCARD_COLOR_0);
		WGFX_CLEAR_MIRROR(DiscardColor1, BGFX_CLEAR_DISCARD_COLOR_1);
		WGFX_CLEAR_MIRROR(DiscardColor2, BGFX_CLEAR_DISCARD_COLOR_2);
		WGFX_CLEAR_MIRROR(DiscardColor3, BGFX_CLEAR_DISCARD_COLOR_3);
		WGFX_CLEAR_MIRROR(DiscardColor4, BGFX_CLEAR_DISCARD_COLOR_4);
		WGFX_CLEAR_MIRROR(DiscardColor5, BGFX_CLEAR_DISCARD_COLOR_5);
		WGFX_CLEAR_MIRROR(DiscardColor6, BGFX_CLEAR_DISCARD_COLOR_6);
		WGFX_CLEAR_MIRROR(DiscardColor7, BGFX_CLEAR_DISCARD_COLOR_7);
		WGFX_CLEAR_MIRROR(DiscardDepth, BGFX_CLEAR_DISCARD_DEPTH);
		WGFX_CLEAR_MIRROR(DiscardStencil, BGFX_CLEAR_DISCARD_STENCIL);
		WGFX_CLEAR_MIRROR(ColorDepth, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH);
		WGFX_CLEAR_MIRROR(ColorDepthStencil,
			BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
		WGFX_CLEAR_MIRROR(DiscardColorMask, BGFX_CLEAR_DISCARD_COLOR_MASK);
		WGFX_CLEAR_MIRROR(DiscardMask, BGFX_CLEAR_DISCARD_MASK);

#undef WGFX_CLEAR_MIRROR

		static_assert(static_cast<u8>(ViewMode::Default) == bgfx::ViewMode::Default,
			"ViewMode::Default must mirror bgfx::ViewMode::Default");
		static_assert(static_cast<u8>(ViewMode::Sequential) == bgfx::ViewMode::Sequential,
			"ViewMode::Sequential must mirror bgfx::ViewMode::Sequential");
		static_assert(static_cast<u8>(ViewMode::DepthAscending) == bgfx::ViewMode::DepthAscending,
			"ViewMode::DepthAscending must mirror bgfx::ViewMode::DepthAscending");
		static_assert(static_cast<u8>(ViewMode::DepthDescending) == bgfx::ViewMode::DepthDescending,
			"ViewMode::DepthDescending must mirror bgfx::ViewMode::DepthDescending");
		static_assert(static_cast<u8>(ViewMode::Count) == bgfx::ViewMode::Count,
			"ViewMode::Count must mirror bgfx::ViewMode::Count");
	} // anonymous namespace
#endif

	View View::create(u16 id)
	{
		View result;

#if defined(WGFX_BACKEND_BGFX)
		if (!detail::device_active() || id == InvalidId)
			return result;

		const bgfx::Caps* caps{ bgfx::getCaps() };
		if (!caps || id >= caps->limits.maxViews)
			return result;

		const u32 generation{ detail::device_generation() };
		if (generation == 0)
			return result;

		result.mId = id;
		result.mGeneration = generation;
#else
		(void)id;
#endif

		return result;
	}

	bool View::valid() const
	{
		return mId != InvalidId
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	u16 View::id() const
	{
		return valid() ? mId : InvalidId;
	}

	bool View::set_rect(
		i16 x,
		i16 y,
		u16 width,
		u16 height,
		float minDepth,
		float maxDepth) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || width == 0 || height == 0)
			return false;

		bgfx::setViewRect(mId, x, y, width, height, minDepth, maxDepth);
		return true;
#else
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		(void)minDepth;
		(void)maxDepth;
		return false;
#endif
	}

	bool View::set_rect(i16 x, i16 y, BackbufferRatio ratio) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || ratio == BackbufferRatio::Explicit)
			return false;

		bgfx::setViewRect(mId, x, y, to_bgfx(ratio));
		return true;
#else
		(void)x;
		(void)y;
		(void)ratio;
		return false;
#endif
	}

	bool View::set_scissor(u16 x, u16 y, u16 width, u16 height) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::setViewScissor(mId, x, y, width, height);
		return true;
#else
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		return false;
#endif
	}

	bool View::set_clear(
		ClearFlags flags,
		u32 rgba,
		float depth,
		u8 stencil) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || 0 != (static_cast<u16>(flags) & ~ClearFlagsMask))
			return false;

		bgfx::setViewClear(
			mId,
			static_cast<u16>(flags),
			rgba,
			depth,
			stencil);
		return true;
#else
		(void)flags;
		(void)rgba;
		(void)depth;
		(void)stencil;
		return false;
#endif
	}

	bool View::set_framebuffer(const FrameBuffer* framebuffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || (framebuffer && !framebuffer->valid()))
			return false;

		const bgfx::FrameBufferHandle handle{ framebuffer
			? detail::FrameBufferAccess::native(*framebuffer)
			: bgfx::FrameBufferHandle{ bgfx::kInvalidHandle } };
		bgfx::setViewFrameBuffer(mId, handle);
		return true;
#else
		(void)framebuffer;
		return false;
#endif
	}

	bool View::set_transform(const void* view, const void* projection) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !view || !projection)
			return false;

		bgfx::setViewTransform(mId, view, projection);
		return true;
#else
		(void)view;
		(void)projection;
		return false;
#endif
	}

	bool View::set_mode(ViewMode mode) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || static_cast<u8>(mode) >= static_cast<u8>(ViewMode::Count))
			return false;

		bgfx::setViewMode(mId, to_bgfx(mode));
		return true;
#else
		(void)mode;
		return false;
#endif
	}

	bool View::set_name(const char* name) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !name)
			return false;

		bgfx::setViewName(mId, name);
		return true;
#else
		(void)name;
		return false;
#endif
	}

	bool View::reset_settings() const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::resetView(mId);
		return true;
#else
		return false;
#endif
	}

	bool View::touch() const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::touch(mId);
		return true;
#else
		return false;
#endif
	}

	bool View::set_state(const RenderState& state) const
	{
		return set_state(state.flags, state.rgba);
	}

	bool View::set_state(RenderStateFlags flags, u32 rgba) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::setState(static_cast<u64>(flags), rgba);
		return true;
#else
		(void)flags;
		(void)rgba;
		return false;
#endif
	}

	bool View::set_vertex_buffer(u8 stream, const VertexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !buffer.valid() || !valid_stream(stream))
			return false;

		bgfx::setVertexBuffer(
			stream,
			detail::VertexBufferAccess::native(buffer));
		return true;
#else
		(void)stream;
		(void)buffer;
		return false;
#endif
	}

	bool View::set_vertex_buffer(
		u8 stream,
		const VertexBuffer& buffer,
		u32 startVertex,
		u32 numVertices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| !buffer.valid()
		|| !valid_stream(stream)
		|| numVertices == 0)
		{
			return false;
		}

		bgfx::setVertexBuffer(
			stream,
			detail::VertexBufferAccess::native(buffer),
			startVertex,
			numVertices);
		return true;
#else
		(void)stream;
		(void)buffer;
		(void)startVertex;
		(void)numVertices;
		return false;
#endif
	}

	bool View::set_vertex_buffer(
		u8 stream,
		const DynamicVertexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !buffer.valid() || !valid_stream(stream))
			return false;

		bgfx::setVertexBuffer(
			stream,
			detail::DynamicVertexBufferAccess::native(buffer));
		return true;
#else
		(void)stream;
		(void)buffer;
		return false;
#endif
	}

	bool View::set_vertex_buffer(
		u8 stream,
		const DynamicVertexBuffer& buffer,
		u32 startVertex,
		u32 numVertices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| !buffer.valid()
		|| !valid_stream(stream)
		|| numVertices == 0)
		{
			return false;
		}

		bgfx::setVertexBuffer(
			stream,
			detail::DynamicVertexBufferAccess::native(buffer),
			startVertex,
			numVertices);
		return true;
#else
		(void)stream;
		(void)buffer;
		(void)startVertex;
		(void)numVertices;
		return false;
#endif
	}

	bool View::set_index_buffer(const IndexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !buffer.valid())
			return false;

		bgfx::setIndexBuffer(detail::IndexBufferAccess::native(buffer));
		return true;
#else
		(void)buffer;
		return false;
#endif
	}

	bool View::set_index_buffer(
		const IndexBuffer& buffer,
		u32 firstIndex,
		u32 numIndices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !buffer.valid() || numIndices == 0)
			return false;

		bgfx::setIndexBuffer(
			detail::IndexBufferAccess::native(buffer),
			firstIndex,
			numIndices);
		return true;
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool View::set_index_buffer(const DynamicIndexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !buffer.valid())
			return false;

		bgfx::setIndexBuffer(
			detail::DynamicIndexBufferAccess::native(buffer));
		return true;
#else
		(void)buffer;
		return false;
#endif
	}

	bool View::set_index_buffer(
		const DynamicIndexBuffer& buffer,
		u32 firstIndex,
		u32 numIndices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !buffer.valid() || numIndices == 0)
			return false;

		bgfx::setIndexBuffer(
			detail::DynamicIndexBufferAccess::native(buffer),
			firstIndex,
			numIndices);
		return true;
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool View::set_texture(
		u8 stage,
		const Uniform& sampler,
		const Texture& texture) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| !valid_stage(stage)
		|| !sampler.valid()
		|| !texture.valid())
		{
			return false;
		}

		bgfx::setTexture(
			stage,
			detail::UniformAccess::native(sampler),
			detail::TextureAccess::native(texture));
		return true;
#else
		(void)stage;
		(void)sampler;
		(void)texture;
		return false;
#endif
	}

	bool View::set_texture(
		u8 stage,
		const Uniform& sampler,
		const Texture& texture,
		SamplerFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| !valid_stage(stage)
		|| !sampler.valid()
		|| !texture.valid())
		{
			return false;
		}

		bgfx::setTexture(
			stage,
			detail::UniformAccess::native(sampler),
			detail::TextureAccess::native(texture),
			static_cast<u32>(flags));
		return true;
#else
		(void)stage;
		(void)sampler;
		(void)texture;
		(void)flags;
		return false;
#endif
	}

	bool View::submit(
		const Program& program,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !program.valid())
			return false;

		bgfx::submit(
			mId,
			detail::ProgramAccess::native(program),
			depth,
			static_cast<u8>(flags));
		return true;
#else
		(void)program;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool View::submit(
		const Program& program,
		const OcclusionQuery& query,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !program.valid() || !query.valid())
			return false;

		bgfx::submit(
			mId,
			detail::ProgramAccess::native(program),
			detail::OcclusionQueryAccess::native(query),
			depth,
			static_cast<u8>(flags));
		return true;
#else
		(void)program;
		(void)query;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool View::discard(DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::discard(static_cast<u8>(flags));
		return true;
#else
		(void)flags;
		return false;
#endif
	}
} // namespace wgfx
