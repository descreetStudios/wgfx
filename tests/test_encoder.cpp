#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <atomic>
#include <thread>
#include <utility>

using namespace wcore;
using namespace test;

#if defined(WGFX_BACKEND_BGFX)

namespace
{
	wgfx::DeviceDesc headless_desc(u16 maxEncoders = 0)
	{
		wgfx::DeviceDesc desc;
		desc.renderer = wgfx::Renderer::Noop;
		desc.width = 320;
		desc.height = 180;
		desc.vsync = false;
		desc.maxEncoders = maxEncoders;
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

	const u8 VertexShader[]
	{
		'V', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		42, 0, 0, 0, // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const u8 FragmentShader[]
	{
		'F', 'S', 'H', 12,
		42, 0, 0, 0, // hash in
		2, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const u8 ComputeShader[]
	{
		'C', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		1, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const float Triangle[]
	{
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};

	const u16 Indices[]{ 0, 1, 2 };

	const float IdentityMatrix[16]
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	wgfx::Program graphics_program()
	{
		const wgfx::Shader vertex{
			wgfx::Shader::create(VertexShader, sizeof(VertexShader)) };
		const wgfx::Shader fragment{
			wgfx::Shader::create(FragmentShader, sizeof(FragmentShader)) };
		return wgfx::Program::create(vertex, fragment);
	}

	wgfx::Program compute_program()
	{
		const wgfx::Shader shader{
			wgfx::Shader::create(ComputeShader, sizeof(ComputeShader)) };
		return wgfx::Program::create_compute(shader);
	}

	wgfx::TextureDesc texture_desc(
		u16 width,
		u16 height,
		wgfx::TextureFlags flags = wgfx::TextureFlags::None)
	{
		wgfx::TextureDesc desc;
		desc.width = width;
		desc.height = height;
		desc.format = wgfx::TextureFormat::RGBA8;
		desc.flags = flags;
		return desc;
	}
} // anonymous namespace

TEST(encoder_requires_active_device)
{
	wgfx::Encoder encoder;
	CHECK(!encoder.valid());

	CHECK(!wgfx::Encoder::create().valid());

	CHECK(!encoder.set_marker("wgfx.encoder"));
	CHECK(!encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(!encoder.set_vertex_count(3));
	CHECK(!encoder.discard());

	encoder.reset();
	CHECK(!encoder.valid());
}

TEST(encoder_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	auto first = wgfx::Encoder::create(true);
	REQUIRE(first.valid());

	wgfx::Encoder moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	auto second = wgfx::Encoder::create(true);
	REQUIRE(second.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(encoder_exposes_limits)
{
	auto device = wgfx::Device::create(headless_desc(2));
	REQUIRE(device != nullptr);

	CHECK_EQ(device->desc().maxEncoders, 2);
	CHECK_EQ(device->caps().maxEncoders, 2);
}

TEST(encoder_records_graphics_commands)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::DynamicVertexBuffer dynamicVertexBuffer{
		wgfx::DynamicVertexBuffer::create(
			layout, Triangle, sizeof(Triangle)) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::DynamicIndexBuffer dynamicIndexBuffer{
		wgfx::DynamicIndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::Texture texture{
		wgfx::Texture::create(texture_desc(4, 4)) };
	const wgfx::Uniform sampler{
		wgfx::Uniform::create("s_encoderGraphics", wgfx::UniformType::Sampler) };
	const wgfx::Program program{ graphics_program() };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(dynamicVertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(dynamicIndexBuffer.valid());
	REQUIRE(texture.valid());
	REQUIRE(sampler.valid());
	REQUIRE(program.valid());

	wgfx::Encoder encoder{ wgfx::Encoder::create(true) };
	REQUIRE(encoder.valid());
	CHECK(!encoder.set_marker(nullptr));

	CHECK(encoder.set_marker("wgfx.encoder"));
	CHECK(encoder.set_model_transform(IdentityMatrix));
	CHECK(encoder.set_draw_scissor(0, 0, 320, 180));
	CHECK(encoder.set_depth_control(-1, 1.0f, 0.01f));
	CHECK(encoder.set_sample_mask(0xffffffffu));
	CHECK(encoder.set_stencil(wgfx::StencilNone));

	const wgfx::RenderState state{
		wgfx::RenderStateFlags::WriteRgb
		| wgfx::RenderStateFlags::WriteA
		| wgfx::RenderStateFlags::DepthTestLess,
		0x11223344 };

	CHECK(encoder.set_state(state));
	CHECK(encoder.set_vertex_buffer(0, vertexBuffer));
	CHECK(encoder.set_index_buffer(indexBuffer));
	CHECK(encoder.set_texture(0, sampler, texture));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(encoder.set_vertex_buffer(0, vertexBuffer, 0, 3));
	CHECK(encoder.set_index_buffer(indexBuffer, 0, 3));
	CHECK(encoder.set_texture(
		0, sampler, texture, wgfx::SamplerFlags::Point));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(encoder.set_vertex_buffer(0, dynamicVertexBuffer));
	CHECK(encoder.set_index_buffer(dynamicIndexBuffer));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(encoder.set_vertex_buffer(0, dynamicVertexBuffer, 0, 3));
	CHECK(encoder.set_index_buffer(dynamicIndexBuffer, 0, 3));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(encoder.set_instance_count(1));
	CHECK(encoder.set_instance_data_buffer(vertexBuffer, 0, 1));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(encoder.set_instance_data_buffer(dynamicVertexBuffer, 0, 1));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	CHECK(encoder.set_vertex_count(3));
	CHECK(encoder.submit(view, program));
	CHECK(encoder.discard());

	CHECK(encoder.touch(view));

	CHECK(device->frame() > 0);
	CHECK_EQ(device->stats().encoders.size(), 1u);
}

TEST(encoder_records_compute_commands)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::Program program{ compute_program() };
	const wgfx::Texture image{
		wgfx::Texture::create(texture_desc(
			8, 8, wgfx::TextureFlags::ComputeWrite)) };
	const wgfx::IndirectBuffer indirect{ wgfx::IndirectBuffer::create(2) };

	REQUIRE(program.valid());
	REQUIRE(image.valid());
	REQUIRE(indirect.valid());

	wgfx::Encoder encoder{ wgfx::Encoder::create(true) };
	REQUIRE(encoder.valid());

	CHECK(encoder.set_image(0, image, 0, wgfx::Access::Write));
	CHECK(encoder.set_image(0, image, 0, 1, 0,
		wgfx::Access::ReadWrite, wgfx::TextureFormat::RGBA8));
	CHECK(encoder.dispatch(view, program, 4, 2, 1));
	CHECK(encoder.dispatch(view, program, indirect, 0, 2));
	CHECK(encoder.touch(view));

	CHECK(device->frame() > 0);
}

TEST(encoder_binds_storage_buffers)
{
	auto device = wgfx::Device::create(headless_desc(4));
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

	wgfx::Encoder encoder{ wgfx::Encoder::create(true) };
	REQUIRE(encoder.valid());

	CHECK(encoder.set_buffer(0, vertexBuffer, wgfx::Access::Read));
	CHECK(encoder.set_buffer(1, indexBuffer, wgfx::Access::Read));
	CHECK(encoder.set_buffer(2, dynamicVertexBuffer, wgfx::Access::ReadWrite));
	CHECK(encoder.set_buffer(3, dynamicIndexBuffer, wgfx::Access::ReadWrite));
	CHECK(encoder.set_buffer(4, indirectBuffer, wgfx::Access::Read));

	CHECK(device->frame() > 0);
}

TEST(encoder_blits_textures)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const u8 pixels[4 * 4 * 4]{};
	const wgfx::Texture source{
		wgfx::Texture::create(texture_desc(4, 4), pixels, sizeof(pixels)) };
	const wgfx::Texture destination{
		wgfx::Texture::create(texture_desc(4, 4,
			wgfx::TextureFlags::BlitDestination)) };
	REQUIRE(source.valid());
	REQUIRE(destination.valid());

	wgfx::TextureRegion dstRegion{};
	dstRegion.texture = &destination;
	dstRegion.width = 2;
	dstRegion.height = 2;

	wgfx::TextureRegion srcRegion{};
	srcRegion.texture = &source;
	srcRegion.width = 2;
	srcRegion.height = 2;

	wgfx::Encoder encoder{ wgfx::Encoder::create(true) };
	REQUIRE(encoder.valid());

	CHECK(encoder.blit(view, dstRegion, srcRegion));
	CHECK(!encoder.blit(wgfx::View{}, dstRegion, srcRegion));

	wgfx::TextureRegion missing{};
	CHECK(!encoder.blit(view, missing, srcRegion));
	CHECK(!encoder.blit(view, dstRegion, dstRegion));

	dstRegion.texture = &source;
	CHECK(!encoder.blit(view, dstRegion, srcRegion));

	CHECK(device->frame() > 0);
}

TEST(encoder_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	const wgfx::View invalidView;
	REQUIRE(view.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::DynamicVertexBuffer dynamicVertexBuffer{
		wgfx::DynamicVertexBuffer::create(
			layout, Triangle, sizeof(Triangle)) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::DynamicIndexBuffer dynamicIndexBuffer{
		wgfx::DynamicIndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::Texture texture{
		wgfx::Texture::create(texture_desc(4, 4)) };
	const wgfx::Uniform sampler{
		wgfx::Uniform::create("s_encoderInvalid", wgfx::UniformType::Sampler) };
	const wgfx::Program program{ graphics_program() };
	const wgfx::OcclusionQuery query{ wgfx::OcclusionQuery::create() };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(dynamicVertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(dynamicIndexBuffer.valid());
	REQUIRE(texture.valid());
	REQUIRE(sampler.valid());
	REQUIRE(program.valid());
	REQUIRE(query.valid());

	wgfx::Encoder encoder{ wgfx::Encoder::create(true) };
	REQUIRE(encoder.valid());

	CHECK(!encoder.set_vertex_buffer(4, vertexBuffer));
	CHECK(!encoder.set_vertex_buffer(4, dynamicVertexBuffer));
	CHECK(!encoder.set_vertex_buffer(0, wgfx::VertexBuffer{}));
	CHECK(!encoder.set_vertex_buffer(0, wgfx::DynamicVertexBuffer{}));
	CHECK(!encoder.set_vertex_buffer(0, vertexBuffer, 0, 0));
	CHECK(!encoder.set_vertex_buffer(0, dynamicVertexBuffer, 0, 0));
	CHECK(!encoder.set_index_buffer(wgfx::IndexBuffer{}));
	CHECK(!encoder.set_index_buffer(wgfx::DynamicIndexBuffer{}));
	CHECK(!encoder.set_index_buffer(indexBuffer, 0, 0));
	CHECK(!encoder.set_index_buffer(dynamicIndexBuffer, 0, 0));
	CHECK(!encoder.set_texture(16, sampler, texture));
	CHECK(!encoder.set_texture(0, wgfx::Uniform{}, texture));
	CHECK(!encoder.set_texture(0, sampler, wgfx::Texture{}));
	CHECK(!encoder.set_image(16, texture, 0, wgfx::Access::Read));
	CHECK(!encoder.set_image(
		0, texture, 0, wgfx::Access::Count));
	CHECK(!encoder.set_buffer(0, vertexBuffer, wgfx::Access::Read, 1, 16));
	CHECK(!encoder.set_buffer(0, vertexBuffer, wgfx::Access::Read, 0, 0));

	CHECK(!encoder.submit(invalidView, program));
	CHECK(!encoder.submit(view, wgfx::Program{}));
	CHECK(!encoder.dispatch(invalidView, program));
	CHECK(!encoder.touch(invalidView));
	CHECK(!encoder.set_condition(wgfx::OcclusionQuery{}, true));
	CHECK(encoder.set_condition(query, true));

	CHECK(device->frame() > 0);
}

TEST(encoder_submits_in_parallel)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	const wgfx::View firstView{ wgfx::View::create(0) };
	const wgfx::View secondView{ wgfx::View::create(1) };
	REQUIRE(firstView.valid());
	REQUIRE(secondView.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::Program program{ graphics_program() };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(program.valid());

	std::atomic<bool> succeeded{ true };

	auto submit = [&](const wgfx::View& view)
	{
		wgfx::Encoder encoder{ wgfx::Encoder::create(true) };
		if (!encoder.valid()
			|| !encoder.set_state(wgfx::RenderStateFlags::WriteRgb)
			|| !encoder.set_vertex_buffer(0, vertexBuffer)
			|| !encoder.set_index_buffer(indexBuffer)
			|| !encoder.submit(view, program)
			|| !encoder.touch(view))
		{
			succeeded = false;
		}
	};

	std::thread first{ submit, firstView };
	std::thread second{ submit, secondView };
	first.join();
	second.join();

	CHECK(succeeded);
	CHECK(device->frame() > 0);
	CHECK(device->stats().encoders.size() >= 1);
	CHECK(device->stats().encoders.size() <= device->caps().maxEncoders);
}

TEST(encoder_requires_active_device_after_shutdown)
{
	auto device = wgfx::Device::create(headless_desc(4));
	REQUIRE(device != nullptr);

	CHECK(wgfx::Encoder::create().valid());

	device->shutdown();
	CHECK(!wgfx::Encoder::create().valid());
	CHECK(device->render_frame(0) == wgfx::RenderFrame::NoContext);
}

#else

TEST(encoder_requires_bgfx_backend)
{
	wgfx::Encoder encoder;
	CHECK(!encoder.valid());
	CHECK(!wgfx::Encoder::create().valid());
	CHECK(!encoder.set_state(wgfx::RenderStateFlags::WriteRgb));
	encoder.reset();
}

#endif
