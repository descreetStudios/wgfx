#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <string>

using namespace wcore;
using namespace test;

#if defined(WGFX_BACKEND_BGFX)

namespace
{
	wgfx::DeviceDesc headless_desc()
	{
		wgfx::DeviceDesc desc;
		desc.renderer = wgfx::Renderer::Noop;
		desc.width = 320;
		desc.height = 180;
		desc.vsync = false;
		return desc;
	}
} // anonymous namespace

TEST(device_creates_headless_noop_context)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(device->valid());
	CHECK(device->renderer() == wgfx::Renderer::Noop);
	CHECK_EQ(std::string(device->renderer_name()), std::string("Noop"));
	CHECK_EQ(device->width(), 320u);
	CHECK_EQ(device->height(), 180u);
	CHECK(!device->vsync());
	CHECK(device->caps().renderer == wgfx::Renderer::Noop);
}

TEST(device_frame_advances)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u32 first{ device->frame() };
	const u32 second{ device->frame() };
	CHECK(second > first);
}

TEST(device_rejects_second_instance)
{
	auto first = wgfx::Device::create(headless_desc());
	REQUIRE(first != nullptr);

	auto second = wgfx::Device::create(headless_desc());
	CHECK(second == nullptr);
	CHECK(first->valid());
}

TEST(device_resize_and_vsync_update_state)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	device->resize(640, 360);
	CHECK_EQ(device->width(), 640u);
	CHECK_EQ(device->height(), 360u);

	device->set_vsync(true);
	CHECK(device->vsync());

	device->resize(0, 0);
	CHECK_EQ(device->width(), 640u);
	CHECK_EQ(device->height(), 360u);
}

TEST(device_reset_reapplies_current_state)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	device->resize(512, 288);
	REQUIRE(device->reset());
	CHECK_EQ(device->width(), 512u);
	CHECK_EQ(device->height(), 288u);

	device->set_vsync(true);
	REQUIRE(device->reset());
	CHECK(device->vsync());
	CHECK_EQ(device->width(), 512u);
	CHECK_EQ(device->height(), 288u);

	CHECK(device->frame() > 0);
}

TEST(device_reset_requires_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	device->shutdown();
	CHECK(!device->reset());
}

TEST(device_shutdown_releases_context)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	device->shutdown();
	CHECK(!device->valid());
	CHECK_EQ(std::string(device->renderer_name()), std::string("none"));

	device->shutdown();
	CHECK(!device->valid());
}

TEST(device_can_be_recreated)
{
	{
		auto device = wgfx::Device::create(headless_desc());
		REQUIRE(device != nullptr);
	}

	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	CHECK(device->valid());
}

#else

TEST(device_requires_bgfx_backend)
{
	auto device = wgfx::Device::create(wgfx::DeviceDesc{});
	CHECK(device == nullptr);
}

#endif
