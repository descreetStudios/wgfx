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
} // anonymous namespace

TEST(vertex_buffer_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer buffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };

	REQUIRE(buffer.valid());
}

TEST(vertex_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	CHECK(!wgfx::VertexBuffer::create(layout, nullptr, sizeof(Triangle)).valid());
	CHECK(!wgfx::VertexBuffer::create(layout, Triangle, 0).valid());

	wgfx::VertexLayout unfinished;
	unfinished.begin()
		.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float);
	CHECK(!wgfx::VertexBuffer::create(unfinished, Triangle, sizeof(Triangle)).valid());

	const wgfx::VertexLayout empty;
	CHECK(!wgfx::VertexBuffer::create(empty, Triangle, sizeof(Triangle)).valid());
}

TEST(vertex_buffer_requires_active_device)
{
	const wgfx::VertexLayout layout{ position_layout() };
	CHECK(!wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)).valid());
}

TEST(vertex_buffer_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto first = wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle));
	auto second = wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::VertexBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(vertex_buffer_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle));
	REQUIRE(buffer.valid());

	device->shutdown();
	CHECK(!buffer.valid());
	CHECK(!wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)).valid());

	// The stale buffer is left with an old generation; reset must be a no-op.
	buffer.reset();
	CHECK(!buffer.valid());
}

TEST(vertex_buffer_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	auto buffer = wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle));
	REQUIRE(buffer.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!buffer.valid());
}

#else

TEST(vertex_buffer_requires_bgfx_backend)
{
	wgfx::VertexLayout layout;
	layout.begin()
		.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float)
		.end();

	const float triangle[]{ 0.0f, 0.5f, 0.0f };
	wgfx::VertexBuffer buffer{
		wgfx::VertexBuffer::create(layout, triangle, sizeof(triangle)) };

	CHECK(!buffer.valid());
	buffer.reset();
}

#endif
