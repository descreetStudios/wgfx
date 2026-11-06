#pragma once

#include <wgfx/indirect_buffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct IndirectBufferAccess
		{
			static bgfx::IndirectBufferHandle native(const IndirectBuffer& buffer)
			{
				return bgfx::IndirectBufferHandle{ buffer.mHandle };
			}
		};
	} // namespace detail
}
#endif
