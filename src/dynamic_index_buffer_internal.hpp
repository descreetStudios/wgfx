#pragma once

#include <wgfx/dynamic_index_buffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct DynamicIndexBufferAccess
		{
			static bgfx::DynamicIndexBufferHandle native(
				const DynamicIndexBuffer& buffer)
			{
				return bgfx::DynamicIndexBufferHandle{ buffer.mHandle };
			}
		};
	} // namespace detail
}
#endif
