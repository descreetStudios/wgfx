#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	inline constexpr u32 VersionMajor{ 0 };
	inline constexpr u32 VersionMinor{ 1 };
	inline constexpr u32 VersionPatch{ 0 };
	inline constexpr const char* VersionString{ "0.1.0" };

	const char* version_string();
}
