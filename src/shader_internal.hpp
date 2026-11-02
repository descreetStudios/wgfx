#pragma once

#include <wgfx/shader.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct ShaderAccess
		{
			static bgfx::ShaderHandle native(const Shader& shader)
			{
				return bgfx::ShaderHandle{ shader.mHandle };
			}
		};
	} // namespace detail
}
#endif
