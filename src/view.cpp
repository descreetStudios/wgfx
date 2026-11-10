#include <wgfx/view.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "draw_common.hpp"
#include "framebuffer_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		constexpr u16 ClearFlagsMask{ 0x1fff };

		bool supports_variable_rate_shading()
		{
			const bgfx::Caps* caps{ bgfx::getCaps() };
			return caps && 0 != (caps->supported & BGFX_CAPS_VARIABLE_RATE_SHADING);
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

#define WGFX_ACCESS_MIRROR(name, bgfxName) \
		static_assert(static_cast<u8>(Access::name) == (bgfxName), \
			"Access::" #name " must mirror " #bgfxName)

		WGFX_ACCESS_MIRROR(Read, bgfx::Access::Read);
		WGFX_ACCESS_MIRROR(Write, bgfx::Access::Write);
		WGFX_ACCESS_MIRROR(ReadWrite, bgfx::Access::ReadWrite);
		WGFX_ACCESS_MIRROR(Count, bgfx::Access::Count);

#undef WGFX_ACCESS_MIRROR

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

#define WGFX_STENCIL_TEST_MIRROR(name, bgfxName) \
		static_assert(static_cast<u32>(StencilTest::name) == (bgfxName), \
			"StencilTest::" #name " must mirror " #bgfxName)

		WGFX_STENCIL_TEST_MIRROR(None, 0u);
		WGFX_STENCIL_TEST_MIRROR(Less, BGFX_STENCIL_TEST_LESS);
		WGFX_STENCIL_TEST_MIRROR(Lequal, BGFX_STENCIL_TEST_LEQUAL);
		WGFX_STENCIL_TEST_MIRROR(Equal, BGFX_STENCIL_TEST_EQUAL);
		WGFX_STENCIL_TEST_MIRROR(Gequal, BGFX_STENCIL_TEST_GEQUAL);
		WGFX_STENCIL_TEST_MIRROR(Greater, BGFX_STENCIL_TEST_GREATER);
		WGFX_STENCIL_TEST_MIRROR(Notequal, BGFX_STENCIL_TEST_NOTEQUAL);
		WGFX_STENCIL_TEST_MIRROR(Never, BGFX_STENCIL_TEST_NEVER);
		WGFX_STENCIL_TEST_MIRROR(Always, BGFX_STENCIL_TEST_ALWAYS);

#undef WGFX_STENCIL_TEST_MIRROR

#define WGFX_STENCIL_OP_MIRROR(name, failS, failZ, passZ) \
		static_assert(static_cast<u32>(StencilOp::name) << 20 == (failS), \
			"StencilOp::" #name " must mirror " #failS); \
		static_assert(static_cast<u32>(StencilOp::name) << 24 == (failZ), \
			"StencilOp::" #name " must mirror " #failZ); \
		static_assert(static_cast<u32>(StencilOp::name) << 28 == (passZ), \
			"StencilOp::" #name " must mirror " #passZ)

		WGFX_STENCIL_OP_MIRROR(Zero,
			BGFX_STENCIL_OP_FAIL_S_ZERO,
			BGFX_STENCIL_OP_FAIL_Z_ZERO,
			BGFX_STENCIL_OP_PASS_Z_ZERO);
		WGFX_STENCIL_OP_MIRROR(Keep,
			BGFX_STENCIL_OP_FAIL_S_KEEP,
			BGFX_STENCIL_OP_FAIL_Z_KEEP,
			BGFX_STENCIL_OP_PASS_Z_KEEP);
		WGFX_STENCIL_OP_MIRROR(Replace,
			BGFX_STENCIL_OP_FAIL_S_REPLACE,
			BGFX_STENCIL_OP_FAIL_Z_REPLACE,
			BGFX_STENCIL_OP_PASS_Z_REPLACE);
		WGFX_STENCIL_OP_MIRROR(Incr,
			BGFX_STENCIL_OP_FAIL_S_INCR,
			BGFX_STENCIL_OP_FAIL_Z_INCR,
			BGFX_STENCIL_OP_PASS_Z_INCR);
		WGFX_STENCIL_OP_MIRROR(IncrSat,
			BGFX_STENCIL_OP_FAIL_S_INCRSAT,
			BGFX_STENCIL_OP_FAIL_Z_INCRSAT,
			BGFX_STENCIL_OP_PASS_Z_INCRSAT);
		WGFX_STENCIL_OP_MIRROR(Decr,
			BGFX_STENCIL_OP_FAIL_S_DECR,
			BGFX_STENCIL_OP_FAIL_Z_DECR,
			BGFX_STENCIL_OP_PASS_Z_DECR);
		WGFX_STENCIL_OP_MIRROR(DecrSat,
			BGFX_STENCIL_OP_FAIL_S_DECRSAT,
			BGFX_STENCIL_OP_FAIL_Z_DECRSAT,
			BGFX_STENCIL_OP_PASS_Z_DECRSAT);
		WGFX_STENCIL_OP_MIRROR(Invert,
			BGFX_STENCIL_OP_FAIL_S_INVERT,
			BGFX_STENCIL_OP_FAIL_Z_INVERT,
			BGFX_STENCIL_OP_PASS_Z_INVERT);

#undef WGFX_STENCIL_OP_MIRROR

		static_assert(StencilNone == BGFX_STENCIL_NONE,
			"StencilNone must mirror BGFX_STENCIL_NONE");

#define WGFX_SHADING_RATE_MIRROR(name, bgfxName) \
		static_assert(static_cast<u8>(ShadingRate::name) == (bgfxName), \
			"ShadingRate::" #name " must mirror " #bgfxName)

		WGFX_SHADING_RATE_MIRROR(Rate1x1, bgfx::ShadingRate::Rate1x1);
		WGFX_SHADING_RATE_MIRROR(Rate1x2, bgfx::ShadingRate::Rate1x2);
		WGFX_SHADING_RATE_MIRROR(Rate2x1, bgfx::ShadingRate::Rate2x1);
		WGFX_SHADING_RATE_MIRROR(Rate2x2, bgfx::ShadingRate::Rate2x2);
		WGFX_SHADING_RATE_MIRROR(Rate2x4, bgfx::ShadingRate::Rate2x4);
		WGFX_SHADING_RATE_MIRROR(Rate4x2, bgfx::ShadingRate::Rate4x2);
		WGFX_SHADING_RATE_MIRROR(Rate4x4, bgfx::ShadingRate::Rate4x4);
		WGFX_SHADING_RATE_MIRROR(Count, bgfx::ShadingRate::Count);

#undef WGFX_SHADING_RATE_MIRROR

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

	bool View::set_depth_bias(i32 constant, float slopeScale, float clamp) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::setViewDepthBias(mId, constant, slopeScale, clamp);
		return true;
#else
		(void)constant;
		(void)slopeScale;
		(void)clamp;
		return false;
#endif
	}

	bool View::set_view_sample_mask(u32 mask) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::setViewSampleMask(mId, mask);
		return true;
#else
		(void)mask;
		return false;
#endif
	}

	bool View::set_shading_rate(ShadingRate rate) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
			|| static_cast<u8>(rate) >= static_cast<u8>(ShadingRate::Count)
			|| !supports_variable_rate_shading())
		{
			return false;
		}

		bgfx::setViewShadingRate(
			mId,
			static_cast<bgfx::ShadingRate::Enum>(rate));
		return true;
#else
		(void)rate;
		return false;
#endif
	}

	bool View::set_view_order(u16 count, const u16* order) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || count == 0)
			return false;

		if (order)
		{
			const bgfx::Caps* caps{ bgfx::getCaps() };
			if (!caps)
				return false;

			const u32 maxViews{ caps->limits.maxViews };
			const u32 remaining{ maxViews > mId ? maxViews - mId : 0 };
			const u32 num{ count < remaining ? count : remaining };

			for (u32 index = 0; index < num; ++index)
			{
				if (order[index] >= maxViews)
					return false;
			}
		}

		bgfx::setViewOrder(mId, count, order);
		return true;
#else
		(void)count;
		(void)order;
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
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_state(
			detail::draw::GlobalTarget{}, valid(), state);
#else
		(void)state;
		return false;
#endif
	}

	bool View::set_state(RenderStateFlags flags, u32 rgba) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_state(
			detail::draw::GlobalTarget{}, valid(), flags, rgba);
#else
		(void)flags;
		(void)rgba;
		return false;
#endif
	}

	bool View::set_vertex_count(u32 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_count(
			detail::draw::GlobalTarget{}, valid(), count);
#else
		(void)count;
		return false;
#endif
	}

	bool View::set_model_transform(const void* matrix, u16 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_model_transform(
			detail::draw::GlobalTarget{}, valid(), matrix, count);
#else
		(void)matrix;
		(void)count;
		return false;
#endif
	}

	bool View::set_draw_scissor(
		u16 x,
		u16 y,
		u16 width,
		u16 height) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_draw_scissor(
			detail::draw::GlobalTarget{}, valid(), x, y, width, height);
#else
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		return false;
#endif
	}

	bool View::set_depth_control(
		i32 constant,
		float slopeScale,
		float clamp,
		bool depthClamp) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_depth_control(
			detail::draw::GlobalTarget{}, valid(),
			constant, slopeScale, clamp, depthClamp);
#else
		(void)constant;
		(void)slopeScale;
		(void)clamp;
		(void)depthClamp;
		return false;
#endif
	}

	bool View::set_sample_mask(u32 mask) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_sample_mask(
			detail::draw::GlobalTarget{}, valid(), mask);
#else
		(void)mask;
		return false;
#endif
	}

	bool View::set_stencil(u32 front, u32 back) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_stencil(
			detail::draw::GlobalTarget{}, valid(), front, back);
#else
		(void)front;
		(void)back;
		return false;
#endif
	}

	bool View::set_vertex_buffer(
		u8 stream,
		const VertexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			detail::draw::GlobalTarget{}, valid(), stream, buffer);
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
		return detail::draw::set_vertex_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stream, buffer, startVertex, numVertices);
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
		return detail::draw::set_vertex_buffer(
			detail::draw::GlobalTarget{}, valid(), stream, buffer);
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
		return detail::draw::set_vertex_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stream, buffer, startVertex, numVertices);
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
		const TransientVertexBuffer& buffer) const
	{
		return set_vertex_buffer(stream, buffer, 0, buffer.count());
	}

	bool View::set_vertex_buffer(
		u8 stream,
		const TransientVertexBuffer& buffer,
		u32 startVertex,
		u32 numVertices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stream, buffer, startVertex, numVertices);
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
		return detail::draw::set_index_buffer(
			detail::draw::GlobalTarget{}, valid(), buffer);
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
		return detail::draw::set_index_buffer(
			detail::draw::GlobalTarget{}, valid(),
			buffer, firstIndex, numIndices);
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
		return detail::draw::set_index_buffer(
			detail::draw::GlobalTarget{}, valid(), buffer);
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
		return detail::draw::set_index_buffer(
			detail::draw::GlobalTarget{}, valid(),
			buffer, firstIndex, numIndices);
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool View::set_index_buffer(const TransientIndexBuffer& buffer) const
	{
		return set_index_buffer(buffer, 0, buffer.count());
	}

	bool View::set_index_buffer(
		const TransientIndexBuffer& buffer,
		u32 firstIndex,
		u32 numIndices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			detail::draw::GlobalTarget{}, valid(),
			buffer, firstIndex, numIndices);
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool View::set_instance_count(u32 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_count(
			detail::draw::GlobalTarget{}, valid(), count);
#else
		(void)count;
		return false;
#endif
	}

	bool View::set_instance_data_buffer(
		const VertexBuffer& buffer,
		u32 startVertex,
		u32 numInstances) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_data_buffer(
			detail::draw::GlobalTarget{}, valid(),
			buffer, startVertex, numInstances);
#else
		(void)buffer;
		(void)startVertex;
		(void)numInstances;
		return false;
#endif
	}

	bool View::set_instance_data_buffer(
		const DynamicVertexBuffer& buffer,
		u32 startVertex,
		u32 numInstances) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_data_buffer(
			detail::draw::GlobalTarget{}, valid(),
			buffer, startVertex, numInstances);
#else
		(void)buffer;
		(void)startVertex;
		(void)numInstances;
		return false;
#endif
	}

	bool View::set_instance_data_buffer(
		const InstanceDataBuffer& buffer,
		u32 start,
		u32 num) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_data_buffer(
			detail::draw::GlobalTarget{}, valid(), buffer, start, num);
#else
		(void)buffer;
		(void)start;
		(void)num;
		return false;
#endif
	}

	bool View::set_texture(
		u8 stage,
		const Uniform& sampler,
		const Texture& texture) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_texture(
			detail::draw::GlobalTarget{}, valid(), stage, sampler, texture);
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
		return detail::draw::set_texture(
			detail::draw::GlobalTarget{}, valid(),
			stage, sampler, texture, flags);
#else
		(void)stage;
		(void)sampler;
		(void)texture;
		(void)flags;
		return false;
#endif
	}

	bool View::set_image(
		u8 stage,
		const Texture& texture,
		u8 mip,
		Access access,
		TextureFormat format) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_image(
			detail::draw::GlobalTarget{}, valid(),
			stage, texture, mip, access, format);
#else
		(void)stage;
		(void)texture;
		(void)mip;
		(void)access;
		(void)format;
		return false;
#endif
	}

	bool View::set_image(
		u8 stage,
		const Texture& texture,
		u16 firstLayer,
		u16 numLayers,
		u8 mip,
		Access access,
		TextureFormat format) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_image(
			detail::draw::GlobalTarget{}, valid(),
			stage, texture, firstLayer, numLayers, mip, access, format);
#else
		(void)stage;
		(void)texture;
		(void)firstLayer;
		(void)numLayers;
		(void)mip;
		(void)access;
		(void)format;
		return false;
#endif
	}

	bool View::set_buffer(
		u8 stage,
		const VertexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool View::set_buffer(
		u8 stage,
		const IndexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool View::set_buffer(
		u8 stage,
		const DynamicVertexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool View::set_buffer(
		u8 stage,
		const DynamicIndexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool View::set_buffer(
		u8 stage,
		const IndirectBuffer& buffer,
		Access access) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			detail::draw::GlobalTarget{}, valid(),
			stage, buffer, access);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		return false;
#endif
	}

	bool View::dispatch(
		const Program& program,
		u32 numX,
		u32 numY,
		u32 numZ,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::dispatch(
			detail::draw::GlobalTarget{}, valid(), mId,
			program, numX, numY, numZ, flags);
#else
		(void)program;
		(void)numX;
		(void)numY;
		(void)numZ;
		(void)flags;
		return false;
#endif
	}

	bool View::dispatch(
		const Program& program,
		const IndirectBuffer& indirect,
		u32 start,
		u32 num,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::dispatch(
			detail::draw::GlobalTarget{}, valid(), mId,
			program, indirect, start, num, flags);
#else
		(void)program;
		(void)indirect;
		(void)start;
		(void)num;
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
		return detail::draw::submit(
			detail::draw::GlobalTarget{}, valid(), mId,
			program, depth, flags);
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
		return detail::draw::submit(
			detail::draw::GlobalTarget{}, valid(), mId,
			program, query, depth, flags);
#else
		(void)program;
		(void)query;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool View::submit(
		const Program& program,
		const IndirectBuffer& indirect,
		u32 start,
		u32 num,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::submit(
			detail::draw::GlobalTarget{}, valid(), mId,
			program, indirect, start, num, depth, flags);
#else
		(void)program;
		(void)indirect;
		(void)start;
		(void)num;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool View::submit(
		const Program& program,
		const IndirectBuffer& indirect,
		u32 start,
		const IndexBuffer& numBuffer,
		u32 numIndex,
		u32 numMax,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::submit(
			detail::draw::GlobalTarget{}, valid(), mId,
			program, indirect, start, numBuffer, numIndex, numMax,
			depth, flags);
#else
		(void)program;
		(void)indirect;
		(void)start;
		(void)numBuffer;
		(void)numIndex;
		(void)numMax;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool View::discard(DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::discard(
			detail::draw::GlobalTarget{}, valid(), flags);
#else
		(void)flags;
		return false;
#endif
	}
}
