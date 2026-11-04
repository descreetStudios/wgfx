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

	wgfx::VertexLayout position_layout()
	{
		wgfx::VertexLayout layout;
		layout.begin()
			.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float)
			.end();
		return layout;
	}

	const float Triangle[]
	{
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};

	const float Quad[]
	{
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
	};
} // anonymous namespace

TEST(dynamic_vertex_buffer_creates_empty)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::DynamicVertexBuffer buffer{
		wgfx::DynamicVertexBuffer::create(layout, 4) };
	CHECK(buffer.valid());
}

TEST(dynamic_vertex_buffer_creates_initialized)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::DynamicVertexBuffer buffer{
		wgfx::DynamicVertexBuffer::create(
			layout, Triangle, sizeof(Triangle)) };
	CHECK(buffer.valid());
}

TEST(dynamic_vertex_buffer_updates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::DynamicVertexBuffer::create(layout, 4);
	REQUIRE(buffer.valid());

	CHECK(buffer.update(Triangle, sizeof(Triangle)));
	CHECK(buffer.update(Quad, sizeof(Quad)));

	// Update the last two vertices.
	CHECK(buffer.update(Triangle + 3, sizeof(Triangle) - 3 * sizeof(float), 1));

	CHECK(!buffer.update(nullptr, sizeof(Triangle)));
	CHECK(!buffer.update(Triangle, 0));
	CHECK(!buffer.update(Triangle, sizeof(Triangle) - 1));
}

TEST(dynamic_vertex_buffer_resizes_when_allowed)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::DynamicVertexBuffer::create(
		layout, 1, wgfx::BufferFlags::AllowResize);
	REQUIRE(buffer.valid());

	CHECK(buffer.update(Triangle, sizeof(Triangle)));
	CHECK(buffer.update(Quad, sizeof(Quad)));

	// Growing from a non-zero start vertex cannot be expressed by bgfx.
	CHECK(!buffer.update(Quad, sizeof(Quad), 1));
}

TEST(dynamic_vertex_buffer_rejects_overflow_without_resize)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::DynamicVertexBuffer::create(layout, 2);
	REQUIRE(buffer.valid());

	CHECK(buffer.update(Triangle, sizeof(Triangle) - 3 * sizeof(float)));
	CHECK(!buffer.update(Triangle, sizeof(Triangle)));

	// Also reject updates that fit inside the buffer but start too late.
	CHECK(!buffer.update(Triangle, 2 * 3 * sizeof(float), 1));
}

TEST(dynamic_vertex_buffer_rejects_compute_write_updates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };

	wgfx::DynamicVertexBuffer buffer{
		wgfx::DynamicVertexBuffer::create(
			layout, 4, wgfx::BufferFlags::ComputeWrite) };
	REQUIRE(buffer.valid());
	CHECK(!buffer.update(Triangle, sizeof(Triangle)));

	CHECK(!wgfx::DynamicVertexBuffer::create(
		layout,
		Triangle,
		sizeof(Triangle),
		wgfx::BufferFlags::ComputeWrite).valid());
}

TEST(dynamic_vertex_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexLayout empty;
	const u8 invalidSize[sizeof(Triangle) - 1]{};

	CHECK(!wgfx::DynamicVertexBuffer::create(layout, 0).valid());
	CHECK(!wgfx::DynamicVertexBuffer::create(empty, 4).valid());
	CHECK(!wgfx::DynamicVertexBuffer::create(layout, nullptr, sizeof(Triangle)).valid());
	CHECK(!wgfx::DynamicVertexBuffer::create(layout, Triangle, 0).valid());
	CHECK(!wgfx::DynamicVertexBuffer::create(
		layout, invalidSize, sizeof(invalidSize)).valid());

	wgfx::VertexLayout unfinished;
	unfinished.begin()
		.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float);
	CHECK(!wgfx::DynamicVertexBuffer::create(unfinished, 4).valid());
}

TEST(dynamic_vertex_buffer_requires_active_device)
{
	const wgfx::VertexLayout layout{ position_layout() };

	CHECK(!wgfx::DynamicVertexBuffer::create(layout, 4).valid());
	CHECK(!wgfx::DynamicVertexBuffer::create(
		layout, Triangle, sizeof(Triangle)).valid());

	wgfx::DynamicVertexBuffer buffer;
	CHECK(!buffer.valid());
	CHECK(!buffer.update(Triangle, sizeof(Triangle)));
}

TEST(dynamic_vertex_buffer_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto first = wgfx::DynamicVertexBuffer::create(layout, 4);
	auto second = wgfx::DynamicVertexBuffer::create(layout, 2);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::DynamicVertexBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());
	CHECK(moved.update(Triangle, sizeof(Triangle)));

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());
	CHECK(second.update(Triangle, sizeof(Triangle)));

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(dynamic_vertex_buffer_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::DynamicVertexBuffer::create(layout, 4);
	REQUIRE(buffer.valid());

	device->shutdown();
	CHECK(!buffer.valid());
	CHECK(!buffer.update(Triangle, sizeof(Triangle)));
	CHECK(!wgfx::DynamicVertexBuffer::create(layout, 4).valid());

	// The stale buffer is left with an old generation; reset must be a no-op.
	buffer.reset();
	CHECK(!buffer.valid());
}

TEST(dynamic_vertex_buffer_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::DynamicVertexBuffer::create(layout, 4);
	REQUIRE(buffer.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!buffer.valid());
}

#else

TEST(dynamic_vertex_buffer_requires_bgfx_backend)
{
	wgfx::VertexLayout layout;
	layout.begin()
		.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float)
		.end();

	const float triangle[]{ 0.0f, 0.5f, 0.0f };
	wgfx::DynamicVertexBuffer buffer{
		wgfx::DynamicVertexBuffer::create(
			layout, triangle, sizeof(triangle)) };

	CHECK(!buffer.valid());
	CHECK(!buffer.update(triangle, sizeof(triangle)));
	buffer.reset();
}

#endif
