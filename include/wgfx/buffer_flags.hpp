#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	enum class BufferFlags : u16
	{
		None = 0x0000,
		ComputeRead = 0x0100,
		ComputeWrite = 0x0200,
		ComputeReadWrite = 0x0300,
		AllowResize = 0x0800,
		Index32 = 0x1000,
	};

	constexpr BufferFlags operator|(BufferFlags lhs, BufferFlags rhs) noexcept
	{
		return static_cast<BufferFlags>(
			static_cast<u16>(lhs) | static_cast<u16>(rhs));
	}

	constexpr BufferFlags operator&(BufferFlags lhs, BufferFlags rhs) noexcept
	{
		return static_cast<BufferFlags>(
			static_cast<u16>(lhs) & static_cast<u16>(rhs));
	}

	constexpr BufferFlags& operator|=(BufferFlags& lhs, BufferFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}
}
