#include <wgfx/backend.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
	const char* backend_name()
	{
#if defined(WGFX_BACKEND_BGFX)
		return "bgfx";
#else
		return "none";
#endif
	}

	u32 backend_api_version()
	{
#if defined(WGFX_BACKEND_BGFX)
		return BGFX_API_VERSION;
#else
		return 0;
#endif
	}
}
