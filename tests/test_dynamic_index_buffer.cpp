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

	const u16 Indices16[]
	{
		0, 1, 2,
		2, 3, 0,
	};

	const u32 Indices32[]
	{
		0, 1, 2,
		2, 3, 0,
	};
} // anonymous namespace

TEST(dynamic_index_buffer_creates_empty)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::DynamicIndexBuffer indices16{
		wgfx::DynamicIndexBuffer::create(4) };
	const wgfx::DynamicIndexBuffer indices32{
		wgfx::DynamicIndexBuffer::create(4, wgfx::IndexType::Uint32) };

	CHECK(indices16.valid());
	CHECK(indices32.valid());
}

TEST(dynamic_index_buffer_creates_initialized)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::DynamicIndexBuffer indices16{
		wgfx::DynamicIndexBuffer::create(Indices16, sizeof(Indices16)) };
	const wgfx::DynamicIndexBuffer indices32{
		wgfx::DynamicIndexBuffer::create(
			Indices32,
			sizeof(Indices32),
			wgfx::IndexType::Uint32) };

	CHECK(indices16.valid());
	CHECK(indices32.valid());
}

TEST(dynamic_index_buffer_updates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::DynamicIndexBuffer::create(6);
	REQUIRE(buffer.valid());

	CHECK(buffer.update(Indices16, sizeof(Indices16)));

	// Update two indices starting at index 1.
	CHECK(buffer.update(Indices16 + 1, 2 * sizeof(u16), 1));

	CHECK(!buffer.update(nullptr, sizeof(Indices16)));
	CHECK(!buffer.update(Indices16, 0));
	CHECK(!buffer.update(Indices16, 3));

	auto buffer32 = wgfx::DynamicIndexBuffer::create(6, wgfx::IndexType::Uint32);
	REQUIRE(buffer32.valid());
	CHECK(buffer32.update(Indices32, sizeof(Indices32)));
}

TEST(dynamic_index_buffer_resizes_when_allowed)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::DynamicIndexBuffer::create(
		1,
		wgfx::IndexType::Uint16,
		wgfx::BufferFlags::AllowResize);
	REQUIRE(buffer.valid());

	CHECK(buffer.update(Indices16, sizeof(Indices16)));
	CHECK(buffer.update(Indices16, sizeof(Indices16)));

	// Growing from a non-zero start index cannot be expressed by bgfx.
	CHECK(!buffer.update(Indices16, sizeof(Indices16), 1));
}

TEST(dynamic_index_buffer_rejects_overflow_without_resize)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::DynamicIndexBuffer::create(2);
	REQUIRE(buffer.valid());

	CHECK(buffer.update(Indices16, 2 * sizeof(u16)));
	CHECK(!buffer.update(Indices16, 3 * sizeof(u16)));

	// Also reject updates that fit inside the buffer but start too late.
	CHECK(!buffer.update(Indices16, 2 * sizeof(u16), 1));
}

TEST(dynamic_index_buffer_rejects_compute_write_updates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::DynamicIndexBuffer buffer{
		wgfx::DynamicIndexBuffer::create(
			4,
			wgfx::IndexType::Uint16,
			wgfx::BufferFlags::ComputeWrite) };
	REQUIRE(buffer.valid());
	CHECK(!buffer.update(Indices16, sizeof(Indices16)));

	CHECK(!wgfx::DynamicIndexBuffer::create(
		Indices16,
		sizeof(Indices16),
		wgfx::IndexType::Uint16,
		wgfx::BufferFlags::ComputeWrite).valid());
}

TEST(dynamic_index_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 invalidSize[sizeof(Indices16) - 1]{};

	CHECK(!wgfx::DynamicIndexBuffer::create(0).valid());
	CHECK(!wgfx::DynamicIndexBuffer::create(
		4, static_cast<wgfx::IndexType>(2)).valid());
	CHECK(!wgfx::DynamicIndexBuffer::create(nullptr, sizeof(Indices16)).valid());
	CHECK(!wgfx::DynamicIndexBuffer::create(Indices16, 0).valid());
	CHECK(!wgfx::DynamicIndexBuffer::create(
		invalidSize, sizeof(invalidSize)).valid());
	CHECK(!wgfx::DynamicIndexBuffer::create(
		Indices16,
		sizeof(Indices16),
		static_cast<wgfx::IndexType>(2)).valid());
}

TEST(dynamic_index_buffer_requires_active_device)
{
	CHECK(!wgfx::DynamicIndexBuffer::create(4).valid());
	CHECK(!wgfx::DynamicIndexBuffer::create(
		Indices16, sizeof(Indices16)).valid());

	wgfx::DynamicIndexBuffer buffer;
	CHECK(!buffer.valid());
	CHECK(!buffer.update(Indices16, sizeof(Indices16)));
}

TEST(dynamic_index_buffer_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::DynamicIndexBuffer::create(6);
	auto second = wgfx::DynamicIndexBuffer::create(
		2, wgfx::IndexType::Uint32);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::DynamicIndexBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());
	CHECK(moved.update(Indices16, sizeof(Indices16)));

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());
	CHECK(second.update(Indices16, sizeof(Indices16)));

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(dynamic_index_buffer_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::DynamicIndexBuffer::create(6);
	REQUIRE(buffer.valid());

	device->shutdown();
	CHECK(!buffer.valid());
	CHECK(!buffer.update(Indices16, sizeof(Indices16)));
	CHECK(!wgfx::DynamicIndexBuffer::create(6).valid());

	// The stale buffer is left with an old generation; reset must be a no-op.
	buffer.reset();
	CHECK(!buffer.valid());
}

TEST(dynamic_index_buffer_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto buffer = wgfx::DynamicIndexBuffer::create(6);
	REQUIRE(buffer.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!buffer.valid());
}

#else

TEST(dynamic_index_buffer_requires_bgfx_backend)
{
	const u16 indices[]{ 0, 1, 2 };

	wgfx::DynamicIndexBuffer buffer{
		wgfx::DynamicIndexBuffer::create(indices, sizeof(indices)) };

	CHECK(!buffer.valid());
	CHECK(!buffer.update(indices, sizeof(indices)));
	buffer.reset();
}

#endif
