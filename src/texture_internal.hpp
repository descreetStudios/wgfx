#pragma once

#include <wgfx/texture.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct TextureAccess
		{
			static bgfx::TextureHandle native(const Texture& texture)
			{
				return bgfx::TextureHandle{ texture.mHandle };
			}
		};
	} // namespace detail
}
#endif
