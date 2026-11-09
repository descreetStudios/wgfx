#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	enum class Access : u8
	{
		Read,
		Write,
		ReadWrite,

		Count,
	};
}
