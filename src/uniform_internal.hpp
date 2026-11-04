#pragma once

#include <wgfx/uniform.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct UniformAccess
		{
			static bgfx::UniformHandle native(const Uniform& uniform)
			{
				return bgfx::UniformHandle{ uniform.mHandle };
			}
		};
	} // namespace detail
}
#endif
