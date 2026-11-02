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

	const u16 kIndices16[]{ 0, 1, 2 };
	const u32 kIndices32[]{ 0, 1, 2 };
} // anonymous namespace

TEST(index_buffer_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::IndexBuffer buffer16{
		wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16)) };
	CHECK(buffer16.valid());

	const wgfx::IndexBuffer buffer32{
		wgfx::IndexBuffer::create(
			kIndices32, sizeof(kIndices32), wgfx::IndexType::Uint32) };
	CHECK(buffer32.valid());
}

TEST(index_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(!wgfx::IndexBuffer::create(nullptr, sizeof(kIndices16)).valid());
	CHECK(!wgfx::IndexBuffer::create(kIndices16, 0).valid());

	const u8 oddSize[5]{};
	CHECK(!wgfx::IndexBuffer::create(oddSize, sizeof(oddSize), wgfx::IndexType::Uint16).valid());
	CHECK(!wgfx::IndexBuffer::create(
		kIndices16, sizeof(kIndices16), wgfx::IndexType::Uint32).valid());

	CHECK(!wgfx::IndexBuffer::create(
		kIndices16,
		sizeof(kIndices16),
		static_cast<wgfx::IndexType>(42)).valid());
}

TEST(index_buffer_requires_active_device)
{
	CHECK(!wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16)).valid());
}

TEST(index_buffer_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16));
	auto second = wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::IndexBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(index_buffer_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16));
	REQUIRE(buffer.valid());

	device->shutdown();
	CHECK(!buffer.valid());
	CHECK(!wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16)).valid());

	// The stale buffer is left with an old generation; reset must be a no-op.
	buffer.reset();
	CHECK(!buffer.valid());
}

TEST(index_buffer_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::IndexBuffer::create(kIndices16, sizeof(kIndices16));
	REQUIRE(buffer.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!buffer.valid());
}

#else

TEST(index_buffer_requires_bgfx_backend)
{
	const u16 indices[]{ 0, 1, 2 };
	wgfx::IndexBuffer buffer{
		wgfx::IndexBuffer::create(indices, sizeof(indices)) };

	CHECK(!buffer.valid());
	buffer.reset();
}

#endif
