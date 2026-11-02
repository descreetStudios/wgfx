#pragma once

#include <wgfx/vertex_buffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct VertexBufferAccess
		{
			static bgfx::VertexBufferHandle native(const VertexBuffer& buffer)
			{
				return bgfx::VertexBufferHandle{ buffer.mHandle };
			}
		};
	} // namespace detail
}
#endif
