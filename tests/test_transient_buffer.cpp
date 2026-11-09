#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

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
			.add(wgfx::VertexAttribute::Position,
				3, wgfx::VertexAttributeType::Float)
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

TEST(transient_vertex_buffer_allocates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	REQUIRE(layout.valid());
	CHECK(layout.stride() != 0);

	constexpr u32 NumVertices{ 3 };
	CHECK_EQ(wgfx::TransientVertexBuffer::available(NumVertices, layout),
		NumVertices);

	wgfx::TransientVertexBuffer buffer{
		wgfx::TransientVertexBuffer::alloc(NumVertices, layout) };
	REQUIRE(buffer.valid());
	REQUIRE(buffer.data() != nullptr);
	CHECK_EQ(buffer.count(), NumVertices);
	CHECK_EQ(buffer.size(), NumVertices * layout.stride());
	CHECK_EQ(buffer.stride(), layout.stride());

	// The allocation is writable for the duration of the frame.
	float* vertices{ static_cast<float*>(buffer.data()) };
	for (u32 i = 0; i < NumVertices * 3; ++i)
		vertices[i] = Triangle[i];
	CHECK(vertices[0] == 0.0f);
	CHECK(vertices[4] == -0.5f);
}

TEST(transient_vertex_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };

	CHECK_EQ(wgfx::TransientVertexBuffer::available(0, layout), 0u);
	CHECK(!wgfx::TransientVertexBuffer::alloc(0, layout).valid());

	wgfx::VertexLayout unfinished;
	unfinished.begin()
		.add(wgfx::VertexAttribute::Position,
			3, wgfx::VertexAttributeType::Float);
	CHECK_EQ(wgfx::TransientVertexBuffer::available(3, unfinished), 0u);
	CHECK(!wgfx::TransientVertexBuffer::alloc(3, unfinished).valid());

	const wgfx::VertexLayout empty;
	CHECK_EQ(wgfx::TransientVertexBuffer::available(3, empty), 0u);
	CHECK(!wgfx::TransientVertexBuffer::alloc(3, empty).valid());

	const u32 maxVertices{
		device->caps().maxTransientVbSize / layout.stride() };
	if (maxVertices != 0)
	{
		CHECK(wgfx::TransientVertexBuffer::available(
			maxVertices + 1, layout) < maxVertices + 1);
		CHECK(!wgfx::TransientVertexBuffer::alloc(
			maxVertices + 1, layout).valid());
	}
}

TEST(transient_index_buffer_allocates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	constexpr u32 NumIndices{ 3 };

	CHECK_EQ(wgfx::TransientIndexBuffer::available(NumIndices), NumIndices);
	wgfx::TransientIndexBuffer buffer16{
		wgfx::TransientIndexBuffer::alloc(NumIndices) };
	REQUIRE(buffer16.valid());
	REQUIRE(buffer16.data() != nullptr);
	CHECK_EQ(buffer16.count(), NumIndices);
	CHECK_EQ(buffer16.size(), NumIndices * 2);
	CHECK(!buffer16.index_32());

	u16* indices16{ static_cast<u16*>(buffer16.data()) };
	indices16[0] = 0;
	indices16[1] = 1;
	indices16[2] = 2;
	CHECK_EQ(indices16[2], 2u);

	CHECK_EQ(wgfx::TransientIndexBuffer::available(NumIndices, true),
		NumIndices);
	wgfx::TransientIndexBuffer buffer32{
		wgfx::TransientIndexBuffer::alloc(NumIndices, true) };
	REQUIRE(buffer32.valid());
	CHECK_EQ(buffer32.count(), NumIndices);
	CHECK_EQ(buffer32.size(), NumIndices * 4);
	CHECK(buffer32.index_32());
}

TEST(transient_index_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK_EQ(wgfx::TransientIndexBuffer::available(0), 0u);
	CHECK(!wgfx::TransientIndexBuffer::alloc(0).valid());

	const u32 maxIndices16{ device->caps().maxTransientIbSize / 2 };
	if (maxIndices16 != 0)
	{
		CHECK(wgfx::TransientIndexBuffer::available(maxIndices16 + 1)
			< maxIndices16 + 1);
		CHECK(!wgfx::TransientIndexBuffer::alloc(maxIndices16 + 1).valid());
	}

	const u32 maxIndices32{ device->caps().maxTransientIbSize / 4 };
	if (maxIndices32 != 0)
	{
		CHECK(wgfx::TransientIndexBuffer::available(
			maxIndices32 + 1, true) < maxIndices32 + 1);
		CHECK(!wgfx::TransientIndexBuffer::alloc(
			maxIndices32 + 1, true).valid());
	}
}

TEST(instance_data_buffer_allocates)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	constexpr u32 NumInstances{ 4 };
	constexpr u16 Stride{ 16 };

	CHECK_EQ(wgfx::InstanceDataBuffer::available(NumInstances, Stride),
		NumInstances);

	wgfx::InstanceDataBuffer buffer{
		wgfx::InstanceDataBuffer::alloc(NumInstances, Stride) };
	REQUIRE(buffer.valid());
	REQUIRE(buffer.data() != nullptr);
	CHECK_EQ(buffer.count(), NumInstances);
	CHECK_EQ(buffer.size(), NumInstances * Stride);
	CHECK_EQ(buffer.stride(), Stride);

	float* instances{ static_cast<float*>(buffer.data()) };
	for (u32 i = 0; i < NumInstances * 4; ++i)
		instances[i] = static_cast<float>(i);
	CHECK(instances[0] == 0.0f);
	CHECK(instances[15] == 15.0f);
}

TEST(instance_data_buffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK_EQ(wgfx::InstanceDataBuffer::available(0, 16), 0u);
	CHECK(!wgfx::InstanceDataBuffer::alloc(0, 16).valid());

	CHECK_EQ(wgfx::InstanceDataBuffer::available(4, 0), 0u);
	CHECK(!wgfx::InstanceDataBuffer::alloc(4, 0).valid());

	// Instance stride must be a multiple of 16.
	CHECK_EQ(wgfx::InstanceDataBuffer::available(4, 15), 0u);
	CHECK(!wgfx::InstanceDataBuffer::alloc(4, 15).valid());

	const u32 maxInstances{ device->caps().maxTransientVbSize / 16u };
	if (maxInstances != 0)
	{
		CHECK(wgfx::InstanceDataBuffer::available(maxInstances + 1, 16)
			< maxInstances + 1);
		CHECK(!wgfx::InstanceDataBuffer::alloc(
			maxInstances + 1, 16).valid());
	}
}

TEST(transient_buffers_bind_to_view)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::TransientVertexBuffer vertices{
		wgfx::TransientVertexBuffer::alloc(3, layout) };
	const wgfx::TransientIndexBuffer indices{
		wgfx::TransientIndexBuffer::alloc(3) };
	const wgfx::InstanceDataBuffer instances{
		wgfx::InstanceDataBuffer::alloc(2, 16) };

	REQUIRE(vertices.valid());
	REQUIRE(indices.valid());
	REQUIRE(instances.valid());

	CHECK(view.set_vertex_buffer(0, vertices));
	CHECK(view.set_vertex_buffer(0, vertices, 1, 2));
	CHECK(view.set_index_buffer(indices));
	CHECK(view.set_index_buffer(indices, 1, 2));
	CHECK(view.set_instance_data_buffer(instances));
	CHECK(view.set_instance_data_buffer(instances, 1, 1));
	CHECK(view.set_instance_data_buffer(instances, 0, 2));

	CHECK(!view.set_vertex_buffer(0, wgfx::TransientVertexBuffer{}));
	CHECK(!view.set_vertex_buffer(4, vertices));
	CHECK(!view.set_vertex_buffer(0, vertices, 0, 0));
	CHECK(!view.set_vertex_buffer(0, vertices, 3, 1));
	CHECK(!view.set_vertex_buffer(0, vertices, 0, 4));

	CHECK(!view.set_index_buffer(wgfx::TransientIndexBuffer{}));
	CHECK(!view.set_index_buffer(indices, 0, 0));
	CHECK(!view.set_index_buffer(indices, 3, 1));
	CHECK(!view.set_index_buffer(indices, 0, 4));

	CHECK(!view.set_instance_data_buffer(wgfx::InstanceDataBuffer{}));
	CHECK(!view.set_instance_data_buffer(instances, 0, 0));
	CHECK(!view.set_instance_data_buffer(instances, 2, 1));
	CHECK(!view.set_instance_data_buffer(instances, 0, 3));

	const wgfx::View invalid;
	CHECK(!invalid.set_vertex_buffer(0, vertices));
	CHECK(!invalid.set_vertex_buffer(0, vertices, 0, 1));
	CHECK(!invalid.set_index_buffer(indices));
	CHECK(!invalid.set_index_buffer(indices, 0, 1));
	CHECK(!invalid.set_instance_data_buffer(instances));
}

TEST(transient_buffers_die_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	wgfx::TransientVertexBuffer vertices{
		wgfx::TransientVertexBuffer::alloc(3, layout) };
	wgfx::TransientIndexBuffer indices{
		wgfx::TransientIndexBuffer::alloc(3) };
	wgfx::InstanceDataBuffer instances{
		wgfx::InstanceDataBuffer::alloc(2, 16) };

	REQUIRE(vertices.valid());
	REQUIRE(indices.valid());
	REQUIRE(instances.valid());

	device->shutdown();

	CHECK(!vertices.valid());
	CHECK(!indices.valid());
	CHECK(!instances.valid());
	CHECK(vertices.data() == nullptr);
	CHECK(indices.data() == nullptr);
	CHECK(instances.data() == nullptr);
	CHECK_EQ(vertices.count(), 0u);
	CHECK_EQ(indices.count(), 0u);
	CHECK_EQ(instances.count(), 0u);

	CHECK_EQ(wgfx::TransientVertexBuffer::available(3, layout), 0u);
	CHECK_EQ(wgfx::TransientIndexBuffer::available(3), 0u);
	CHECK_EQ(wgfx::InstanceDataBuffer::available(2, 16), 0u);
	CHECK(!wgfx::TransientVertexBuffer::alloc(3, layout).valid());
	CHECK(!wgfx::TransientIndexBuffer::alloc(3).valid());
	CHECK(!wgfx::InstanceDataBuffer::alloc(2, 16).valid());
}

#else

TEST(transient_buffer_requires_bgfx_backend)
{
	wgfx::VertexLayout layout;
	layout.begin()
		.add(wgfx::VertexAttribute::Position,
			3, wgfx::VertexAttributeType::Float)
		.end();

	CHECK_EQ(wgfx::TransientVertexBuffer::available(3, layout), 0u);
	CHECK(!wgfx::TransientVertexBuffer::alloc(3, layout).valid());

	CHECK_EQ(wgfx::TransientIndexBuffer::available(3), 0u);
	CHECK(!wgfx::TransientIndexBuffer::alloc(3).valid());
	CHECK_EQ(wgfx::TransientIndexBuffer::available(3, true), 0u);
	CHECK(!wgfx::TransientIndexBuffer::alloc(3, true).valid());

	CHECK_EQ(wgfx::InstanceDataBuffer::available(2, 16), 0u);
	CHECK(!wgfx::InstanceDataBuffer::alloc(2, 16).valid());

	const wgfx::TransientVertexBuffer vertices;
	const wgfx::TransientIndexBuffer indices;
	const wgfx::InstanceDataBuffer instances;

	CHECK(!vertices.valid());
	CHECK(!indices.valid());
	CHECK(!instances.valid());
	CHECK(vertices.data() == nullptr);
	CHECK(indices.data() == nullptr);
	CHECK(instances.data() == nullptr);
	CHECK_EQ(vertices.count(), 0u);
	CHECK_EQ(indices.count(), 0u);
	CHECK_EQ(instances.count(), 0u);
}

#endif
