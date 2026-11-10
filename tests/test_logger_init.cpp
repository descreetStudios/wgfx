#include <wcore/logger.hpp>

#if defined(WGFX_BACKEND_BGFX)
namespace
{
	struct LoggerBootstrap
	{
		LoggerBootstrap()
		{
			wcore::logger::init(wcore::logger::Level::Off);
		}
	};

	[[maybe_unused]] const LoggerBootstrap LoggerBootstrap;
} // anonymous namespace
#endif
