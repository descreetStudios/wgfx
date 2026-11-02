#pragma once

#include <wgfx/vertex_layout.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

#include <new>

namespace wgfx
{
	namespace detail
	{
		struct VertexLayoutAccess
		{
			static bgfx::VertexLayout& native(VertexLayout& layout)
			{
				return *std::launder(
					reinterpret_cast<bgfx::VertexLayout*>(layout.mStorage));
			}

			static const bgfx::VertexLayout& native(const VertexLayout& layout)
			{
				return *std::launder(
					reinterpret_cast<const bgfx::VertexLayout*>(layout.mStorage));
			}
		};
	} // namespace detail
}
#endif
