#pragma once

#include <wgfx/occlusion_query.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct OcclusionQueryAccess
		{
			static bgfx::OcclusionQueryHandle native(
				const OcclusionQuery& query)
			{
				return bgfx::OcclusionQueryHandle{ query.mHandle };
			}
		};
	} // namespace detail
}
#endif
