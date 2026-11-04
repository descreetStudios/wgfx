#pragma once

#include <wgfx/framebuffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct FrameBufferAccess
		{
			static bgfx::FrameBufferHandle native(const FrameBuffer& framebuffer)
			{
				return bgfx::FrameBufferHandle{ framebuffer.mHandle };
			}
		};
	} // namespace detail
}
#endif
