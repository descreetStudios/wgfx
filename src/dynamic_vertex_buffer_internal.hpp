#pragma once

#include <wgfx/dynamic_vertex_buffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct DynamicVertexBufferAccess
		{
			static bgfx::DynamicVertexBufferHandle native(
				const DynamicVertexBuffer& buffer)
			{
				return bgfx::DynamicVertexBufferHandle{ buffer.mHandle };
			}
		};
	} // namespace detail
}
#endif
