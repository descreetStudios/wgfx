#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <string>

using namespace wcore;
using namespace test;

TEST(version_is_set)
{
	CHECK(!std::string(wgfx::VersionString).empty());
	CHECK_EQ(std::string(wgfx::version_string()), std::string(wgfx::VersionString));
}

TEST(backend_is_configured)
{
#if defined(WGFX_BACKEND_BGFX)
	CHECK_EQ(std::string(wgfx::backend_name()), std::string("bgfx"));
	CHECK(wgfx::backend_api_version() > 0);
#else
	CHECK_EQ(std::string(wgfx::backend_name()), std::string("none"));
	CHECK_EQ(wgfx::backend_api_version(), 0u);
#endif
}
