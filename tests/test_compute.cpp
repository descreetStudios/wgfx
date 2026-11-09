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

	const u8 ComputeShader[]
	{
		'C', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		1, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

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

	const u16 Indices[]{ 0, 1, 2 };

	wgfx::Program compute_program()
	{
		const wgfx::Shader shader{
			wgfx::Shader::create(ComputeShader, sizeof(ComputeShader)) };
		return wgfx::Program::create_compute(shader);
	}

	wgfx::Texture compute_texture()
	{
		wgfx::TextureDesc desc;
		desc.width = 8;
		desc.height = 8;
		desc.format = wgfx::TextureFormat::RGBA8;
		desc.flags = wgfx::TextureFlags::ComputeWrite;
		return wgfx::Texture::create(desc);
	}
} // anonymous namespace

TEST(compute_dispatches_with_images)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::Program program{ compute_program() };
	const wgfx::Texture image{ compute_texture() };
	REQUIRE(program.valid());
	REQUIRE(image.valid());

	CHECK(view.set_image(0, image, 0, wgfx::Access::Write));
	CHECK(view.set_image(0, image, 0, 1, 0,
		wgfx::Access::ReadWrite, wgfx::TextureFormat::RGBA8));
	CHECK(view.dispatch(program, 4, 2, 1));
	CHECK(view.touch());
	CHECK(view.discard());
}

TEST(compute_binds_storage_buffers)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle),
			wgfx::BufferFlags::ComputeRead) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices),
			wgfx::IndexType::Uint16, wgfx::BufferFlags::ComputeRead) };
	const wgfx::DynamicVertexBuffer dynamicVertexBuffer{
		wgfx::DynamicVertexBuffer::create(layout, 3,
			wgfx::BufferFlags::ComputeReadWrite) };
	const wgfx::DynamicIndexBuffer dynamicIndexBuffer{
		wgfx::DynamicIndexBuffer::create(3, wgfx::IndexType::Uint16,
			wgfx::BufferFlags::ComputeReadWrite) };
	const wgfx::IndirectBuffer indirectBuffer{
		wgfx::IndirectBuffer::create(2) };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(dynamicVertexBuffer.valid());
	REQUIRE(dynamicIndexBuffer.valid());
	REQUIRE(indirectBuffer.valid());

	CHECK(view.set_buffer(0, vertexBuffer, wgfx::Access::Read));
	CHECK(view.set_buffer(1, indexBuffer, wgfx::Access::Read));
	CHECK(view.set_buffer(2, dynamicVertexBuffer, wgfx::Access::ReadWrite));
	CHECK(view.set_buffer(3, dynamicIndexBuffer, wgfx::Access::Write));
	CHECK(view.set_buffer(4, indirectBuffer, wgfx::Access::Write));
}

TEST(compute_dispatches_indirect)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	const wgfx::Program program{ compute_program() };
	const wgfx::IndirectBuffer indirect{ wgfx::IndirectBuffer::create(3) };

	REQUIRE(view.valid());
	REQUIRE(program.valid());
	REQUIRE(indirect.valid());

	CHECK(view.dispatch(program, indirect));
	CHECK(view.dispatch(program, indirect, 1, 2));
	CHECK(view.dispatch(program, indirect, 3, 0) == false);

	CHECK(!view.dispatch(program, indirect, 0, 0));
	CHECK(!view.dispatch(program, indirect, 3, 1));
	CHECK(!view.dispatch(program, indirect, 1, 3));
	CHECK(!view.dispatch(program, indirect, 4, 0));
}

TEST(compute_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::Program program{ compute_program() };
	const wgfx::Texture image{ compute_texture() };
	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::IndirectBuffer indirect{ wgfx::IndirectBuffer::create(1) };

	REQUIRE(program.valid());
	REQUIRE(image.valid());
	REQUIRE(vertexBuffer.valid());
	REQUIRE(indirect.valid());

	CHECK(!view.set_image(16, image, 0, wgfx::Access::Write));
	CHECK(!view.set_image(0, wgfx::Texture{}, 0, wgfx::Access::Write));
	CHECK(!view.set_image(0, image, 0,
		static_cast<wgfx::Access>(wgfx::Access::Count)));
	CHECK(!view.set_image(0, image, 0, wgfx::Access::Write,
		static_cast<wgfx::TextureFormat>(
			static_cast<u16>(wgfx::TextureFormat::Count) + 1)));
	CHECK(!view.set_image(0, image, 0, 0, 0, wgfx::Access::Write));

	CHECK(!view.set_buffer(16, vertexBuffer, wgfx::Access::Read));
	CHECK(!view.set_buffer(0, wgfx::VertexBuffer{}, wgfx::Access::Read));
	CHECK(!view.set_buffer(0, vertexBuffer,
		static_cast<wgfx::Access>(wgfx::Access::Count)));
	CHECK(!view.set_buffer(0, vertexBuffer, wgfx::Access::Read, 128));
	CHECK(!view.set_buffer(0, vertexBuffer, wgfx::Access::Read, 0, 0));
	CHECK(!view.set_buffer(0, wgfx::IndirectBuffer{}, wgfx::Access::Write));

	CHECK(!view.dispatch(wgfx::Program{}));
	CHECK(!view.dispatch(program, wgfx::IndirectBuffer{}));

	const wgfx::View invalid;
	CHECK(!invalid.dispatch(program));
	CHECK(!invalid.dispatch(program, indirect));
	CHECK(!invalid.set_image(0, image, 0, wgfx::Access::Write));
	CHECK(!invalid.set_buffer(0, vertexBuffer, wgfx::Access::Read));
}

TEST(compute_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	const wgfx::Program program{ compute_program() };
	const wgfx::Texture image{ compute_texture() };
	REQUIRE(view.valid());
	REQUIRE(program.valid());
	REQUIRE(image.valid());

	device->shutdown();

	CHECK(!view.valid());
	CHECK(!program.valid());
	CHECK(!image.valid());
	CHECK(!view.dispatch(program));
	CHECK(!view.set_image(0, image, 0, wgfx::Access::Write));
}

#else

TEST(compute_requires_bgfx_backend)
{
	const wgfx::View view;

	CHECK(!view.dispatch(wgfx::Program{}));
	CHECK(!view.set_image(0, wgfx::Texture{}, 0, wgfx::Access::Write));
	CHECK(!view.set_buffer(0, wgfx::VertexBuffer{}, wgfx::Access::Read));
	CHECK(!view.set_buffer(0, wgfx::IndirectBuffer{}, wgfx::Access::Write));
}

#endif
