#pragma once

#include <wgfx/device.hpp>
#include <wgfx/texture.hpp>

namespace wgfx
{
	enum class SwapChainFlags : u32
	{
		None = 0x00000000,
		Fullscreen = 0x00000001,
		MsaaX2 = 0x00000010,
		MsaaX4 = 0x00000020,
		MsaaX8 = 0x00000030,
		MsaaX16 = 0x00000040,
		SrgbBackbuffer = 0x00008000,
		Hdr10 = 0x00010000,
		Hidpi = 0x00020000,
		TransparentBackbuffer = 0x00100000,
	};

	constexpr SwapChainFlags operator|(SwapChainFlags lhs, SwapChainFlags rhs) noexcept
	{
		return static_cast<SwapChainFlags>(
			static_cast<u32>(lhs) | static_cast<u32>(rhs));
	}

	constexpr SwapChainFlags operator&(SwapChainFlags lhs, SwapChainFlags rhs) noexcept
	{
		return static_cast<SwapChainFlags>(
			static_cast<u32>(lhs) & static_cast<u32>(rhs));
	}

	constexpr SwapChainFlags& operator|=(SwapChainFlags& lhs, SwapChainFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	struct SwapChainDesc
	{
		NativeWindow window{};
		u32 width{ 0 };
		u32 height{ 0 };
		SwapChainFlags flags{ SwapChainFlags::None };
		TextureFormat colorFormat{ TextureFormat::RGBA8 };

		// TextureFormat::Count disables the depth-stencil attachment.
		TextureFormat depthFormat{ TextureFormat::Count };
		u8 numBackBuffers{ 1 };
		u8 maxFrameLatency{ 0 };
	};
} // namespace wgfx
