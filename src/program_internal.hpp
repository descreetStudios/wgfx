#pragma once

#include <wgfx/program.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct ProgramAccess
		{
			static bgfx::ProgramHandle native(const Program& program)
			{
				return bgfx::ProgramHandle{ program.mHandle };
			}
		};
	} // namespace detail
}
#endif
