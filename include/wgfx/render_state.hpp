#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	// Bit flags matching bgfx's BGFX_STATE_* values.
	enum class RenderStateFlags : u64
	{
		None = 0x0000000000000000,

		WriteR = 0x0000000000000001,
		WriteG = 0x0000000000000002,
		WriteB = 0x0000000000000004,
		WriteA = 0x0000000000000008,
		WriteZ = 0x0000004000000000,

		WriteRgb = 0x0000000000000007,
		WriteMask = 0x000000400000000f,

		DepthTestLess = 0x0000000000000010,
		DepthTestLequal = 0x0000000000000020,
		DepthTestEqual = 0x0000000000000030,
		DepthTestGequal = 0x0000000000000040,
		DepthTestGreater = 0x0000000000000050,
		DepthTestNotequal = 0x0000000000000060,
		DepthTestNever = 0x0000000000000070,
		DepthTestAlways = 0x0000000000000080,

		BlendZero = 0x0000000000001000,
		BlendOne = 0x0000000000002000,
		BlendSrcColor = 0x0000000000003000,
		BlendInvSrcColor = 0x0000000000004000,
		BlendSrcAlpha = 0x0000000000005000,
		BlendInvSrcAlpha = 0x0000000000006000,
		BlendDstAlpha = 0x0000000000007000,
		BlendInvDstAlpha = 0x0000000000008000,
		BlendDstColor = 0x0000000000009000,
		BlendInvDstColor = 0x000000000000a000,
		BlendSrcAlphaSat = 0x000000000000b000,
		BlendFactor = 0x000000000000c000,
		BlendInvFactor = 0x000000000000d000,

		BlendEquationAdd = 0x0000000000000000,
		BlendEquationSub = 0x0000000010000000,
		BlendEquationRevSub = 0x0000000020000000,
		BlendEquationMin = 0x0000000030000000,
		BlendEquationMax = 0x0000000040000000,

		CullCw = 0x0000001000000000,
		CullCcw = 0x0000002000000000,

		PrimitiveTriStrip = 0x0001000000000000,
		PrimitiveLines = 0x0002000000000000,
		PrimitiveLineStrip = 0x0003000000000000,
		PrimitivePoints = 0x0004000000000000,

		Msaa = 0x0100000000000000,
		LineAa = 0x0200000000000000,
		ConservativeRaster = 0x0400000000000000,
		FrontCcw = 0x0000008000000000,
		BlendIndependent = 0x0000000400000000,
		BlendAlphaToCoverage = 0x0000000800000000,
	};

	constexpr RenderStateFlags operator|(RenderStateFlags lhs, RenderStateFlags rhs) noexcept
	{
		return static_cast<RenderStateFlags>(
			static_cast<u64>(lhs) | static_cast<u64>(rhs));
	}

	constexpr RenderStateFlags operator&(RenderStateFlags lhs, RenderStateFlags rhs) noexcept
	{
		return static_cast<RenderStateFlags>(
			static_cast<u64>(lhs) & static_cast<u64>(rhs));
	}

	constexpr RenderStateFlags& operator|=(RenderStateFlags& lhs, RenderStateFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr RenderStateFlags& operator&=(RenderStateFlags& lhs, RenderStateFlags rhs) noexcept
	{
		lhs = lhs & rhs;
		return lhs;
	}

	enum class BlendFactor : u64
	{
		Zero = 0x0000000000001000,
		One = 0x0000000000002000,
		SrcColor = 0x0000000000003000,
		InvSrcColor = 0x0000000000004000,
		SrcAlpha = 0x0000000000005000,
		InvSrcAlpha = 0x0000000000006000,
		DstAlpha = 0x0000000000007000,
		InvDstAlpha = 0x0000000000008000,
		DstColor = 0x0000000000009000,
		InvDstColor = 0x000000000000a000,
		SrcAlphaSat = 0x000000000000b000,
		Factor = 0x000000000000c000,
		InvFactor = 0x000000000000d000,
	};

	enum class BlendEquation : u64
	{
		Add = 0x0000000000000000,
		Sub = 0x0000000010000000,
		RevSub = 0x0000000020000000,
		Min = 0x0000000030000000,
		Max = 0x0000000040000000,
	};

	constexpr RenderStateFlags belnd_func_separate(
		BlendFactor srcRgb,
		BlendFactor dstRgb,
		BlendFactor srcAlpha,
		BlendFactor dstAlpha) noexcept
	{
		const u64 rgb{ static_cast<u64>(srcRgb)
			| (static_cast<u64>(dstRgb) << 4) };
		const u64 alpha{ static_cast<u64>(srcAlpha)
			| (static_cast<u64>(dstAlpha) << 4) };
		return static_cast<RenderStateFlags>(rgb | (alpha << 8));
	}

	constexpr RenderStateFlags blend_func(
		BlendFactor src,
		BlendFactor dst) noexcept
	{
		return belnd_func_separate(src, dst, src, dst);
	}

	constexpr RenderStateFlags blend_equation_separate(
		BlendEquation rgb,
		BlendEquation alpha) noexcept
	{
		return static_cast<RenderStateFlags>(
			static_cast<u64>(rgb) | (static_cast<u64>(alpha) << 3));
	}

	constexpr RenderStateFlags blend_equation(BlendEquation equation) noexcept
	{
		return blend_equation_separate(equation, equation);
	}

	constexpr RenderStateFlags alpha_ref(u8 value) noexcept
	{
		return static_cast<RenderStateFlags>(
			static_cast<u64>(value) << 40);
	}

	constexpr RenderStateFlags point_size(u8 value) noexcept
	{
		return static_cast<RenderStateFlags>(
			(static_cast<u64>(value) & 0x0f) << 52);
	}

	inline constexpr RenderStateFlags BlendAdd{
		blend_func(BlendFactor::One, BlendFactor::One) };
	inline constexpr RenderStateFlags BlendAlpha{
		blend_func(BlendFactor::SrcAlpha, BlendFactor::InvSrcAlpha) };
	inline constexpr RenderStateFlags BlendNormal{
		blend_func(BlendFactor::One, BlendFactor::InvSrcAlpha) };
	inline constexpr RenderStateFlags BlendMultiply{
		blend_func(BlendFactor::DstColor, BlendFactor::Zero) };
	inline constexpr RenderStateFlags BlendScreen{
		blend_func(BlendFactor::One, BlendFactor::InvSrcColor) };
	inline constexpr RenderStateFlags BlendDarken{
		blend_func(BlendFactor::One, BlendFactor::One)
		| blend_equation(BlendEquation::Min) };
	inline constexpr RenderStateFlags BlendLighten{
		blend_func(BlendFactor::One, BlendFactor::One)
		| blend_equation(BlendEquation::Max) };

	struct RenderState
	{
		RenderStateFlags flags{
			RenderStateFlags::WriteRgb
			| RenderStateFlags::WriteA
			| RenderStateFlags::WriteZ
			| RenderStateFlags::DepthTestLess
			| RenderStateFlags::CullCw
			| RenderStateFlags::Msaa };

		u32 rgba{ 0 };
	};

	// Bit flags matching bgfx's BGFX_SAMPLER_* values.
	enum class SamplerFlags : u32
	{
		None = 0x00000000,

		UMirror = 0x00000001,
		UClamp = 0x00000002,
		UBorder = 0x00000003,
		VMirror = 0x00000004,
		VClamp = 0x00000008,
		VBorder = 0x0000000c,
		WMirror = 0x00000010,
		WClamp = 0x00000020,
		WBorder = 0x00000030,

		MinPoint = 0x00000040,
		MinAnisotropic = 0x00000080,
		MagPoint = 0x00000100,
		MagAnisotropic = 0x00000200,
		MipPoint = 0x00000400,

		CompareLess = 0x00010000,
		CompareLequal = 0x00020000,
		CompareEqual = 0x00030000,
		CompareGequal = 0x00040000,
		CompareGreater = 0x00050000,
		CompareNotequal = 0x00060000,
		CompareNever = 0x00070000,
		CompareAlways = 0x00080000,

		SampleStencil = 0x00100000,
		Srgb = 0x00200000,

		Point = 0x00000540,
		UvwMirror = 0x00000015,
		UvwClamp = 0x0000002a,
		UvwBorder = 0x0000003f,
	};

	constexpr SamplerFlags operator|(SamplerFlags lhs, SamplerFlags rhs) noexcept
	{
		return static_cast<SamplerFlags>(
			static_cast<u32>(lhs) | static_cast<u32>(rhs));
	}

	constexpr SamplerFlags operator&(SamplerFlags lhs, SamplerFlags rhs) noexcept
	{
		return static_cast<SamplerFlags>(
			static_cast<u32>(lhs) & static_cast<u32>(rhs));
	}

	constexpr SamplerFlags& operator|=(SamplerFlags& lhs, SamplerFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	constexpr SamplerFlags border_color(u8 index) noexcept
	{
		return static_cast<SamplerFlags>(
			(static_cast<u32>(index) & 0x0f) << 24);
	}

	// Bit flags matching bgfx's BGFX_DISCARD_* values.
	enum class DiscardFlags : u8
	{
		None = 0x00,
		Bindings = 0x01,
		IndexBuffer = 0x02,
		InstanceData = 0x04,
		State = 0x08,
		Transform = 0x10,
		VertexStreams = 0x20,
		All = 0xff,
	};

	constexpr DiscardFlags operator|(DiscardFlags lhs, DiscardFlags rhs) noexcept
	{
		return static_cast<DiscardFlags>(
			static_cast<u8>(lhs) | static_cast<u8>(rhs));
	}

	constexpr DiscardFlags operator&(DiscardFlags lhs, DiscardFlags rhs) noexcept
	{
		return static_cast<DiscardFlags>(
			static_cast<u8>(lhs) & static_cast<u8>(rhs));
	}

	constexpr DiscardFlags& operator|=(DiscardFlags& lhs, DiscardFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}
}