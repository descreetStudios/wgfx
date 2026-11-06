#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <utility>

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

TEST(indirect_buffer_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::IndirectBuffer buffer{ wgfx::IndirectBuffer::create(4) };
	REQUIRE(buffer.valid());
	CHECK(buffer.count() == 4);
}

TEST(indirect_buffer_rejects_empty_count)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(!wgfx::IndirectBuffer::create(0).valid());
}

TEST(indirect_buffer_requires_active_device)
{
	CHECK(!wgfx::IndirectBuffer::create(1).valid());

	const wgfx::IndirectBuffer buffer;
	CHECK(!buffer.valid());
	CHECK(buffer.count() == 0);
}

TEST(indirect_buffer_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::IndirectBuffer::create(2);
	auto second = wgfx::IndirectBuffer::create(3);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::IndirectBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(first.count() == 0);
	CHECK(moved.valid());
	CHECK(moved.count() == 2);

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());
	CHECK(second.count() == 2);

	second.reset();
	CHECK(!second.valid());
	CHECK(second.count() == 0);
	second.reset();
}

TEST(indirect_buffer_instances_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::IndirectBuffer::create(1);
	auto second = wgfx::IndirectBuffer::create(1);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(indirect_buffer_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::IndirectBuffer::create(2);
	REQUIRE(buffer.valid());

	device->shutdown();
	CHECK(!buffer.valid());
	CHECK(buffer.count() == 0);
	CHECK(!wgfx::IndirectBuffer::create(2).valid());

	// The stale buffer is left with an old generation; reset must be a no-op.
	buffer.reset();
	CHECK(!buffer.valid());
}

TEST(indirect_buffer_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::IndirectBuffer::create(1);
	REQUIRE(buffer.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!buffer.valid());
}

#else

TEST(indirect_buffer_requires_bgfx_backend)
{
	wgfx::IndirectBuffer buffer{ wgfx::IndirectBuffer::create(1) };

	CHECK(!buffer.valid());
	CHECK(buffer.count() == 0);
	buffer.reset();
}

#endif
