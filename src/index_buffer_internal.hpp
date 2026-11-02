#pragma once

#include <wgfx/index_buffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct IndexBufferAccess
		{
			static bgfx::IndexBufferHandle native(const IndexBuffer& buffer)
			{
				return bgfx::IndexBufferHandle{ buffer.mHandle };
			}
		};
	} // namespace detail
}
#endif
