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

			static bgfx::TextureRegion region(const TextureRegion& region)
			{
				return bgfx::TextureRegion{
					native(*region.texture),
					region.mip, region.x, region.y, region.z,
					region.width, region.height, region.depth,
				};
			}

			static bool blit_valid(
				const TextureRegion& dst,
				const TextureRegion& src);
		};
	} // namespace detail
}
#endif
