#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <utility>

using namespace wcore;
using namespace test;

namespace
{
	const float IdentityMatrix[16]
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};

	const u8 ViewVertexShader[]
	{
		'V', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		1, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const u8 ViewFragmentShader[]
	{
		'F', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		1, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};
} // anonymous namespace

TEST(render_state_composes)
{
	const wgfx::RenderStateFlags state{
		wgfx::RenderStateFlags::WriteRgb
		| wgfx::RenderStateFlags::DepthTestLess
		| wgfx::RenderStateFlags::CullCw };

	CHECK((state & wgfx::RenderStateFlags::WriteR) == wgfx::RenderStateFlags::WriteR);
	CHECK((state & wgfx::RenderStateFlags::WriteA) == wgfx::RenderStateFlags::None);
	CHECK((state & wgfx::RenderStateFlags::DepthTestLess)
		== wgfx::RenderStateFlags::DepthTestLess);

	wgfx::RenderStateFlags mutableState{ wgfx::RenderStateFlags::WriteRgb };
	mutableState |= wgfx::RenderStateFlags::WriteZ;
	CHECK((mutableState & wgfx::RenderStateFlags::WriteZ)
		== wgfx::RenderStateFlags::WriteZ);

	CHECK(wgfx::blend_func(wgfx::BlendFactor::SrcAlpha, wgfx::BlendFactor::InvSrcAlpha)
		== wgfx::BlendAlpha);
	CHECK(wgfx::blend_func(wgfx::BlendFactor::One, wgfx::BlendFactor::One)
		== wgfx::BlendAdd);
	const u64 subEquation{ static_cast<u64>(
		wgfx::RenderStateFlags::BlendEquationSub) };
	CHECK(wgfx::blend_equation(wgfx::BlendEquation::Sub)
		== static_cast<wgfx::RenderStateFlags>(subEquation | (subEquation << 3)));
	CHECK(wgfx::alpha_ref(0x80) == static_cast<wgfx::RenderStateFlags>(0x80ull << 40));
	CHECK(wgfx::point_size(7) == static_cast<wgfx::RenderStateFlags>(7ull << 52));

	const wgfx::SamplerFlags sampler{
		wgfx::SamplerFlags::UvwClamp
		| wgfx::SamplerFlags::Point
		| wgfx::SamplerFlags::Srgb };
	CHECK((sampler & wgfx::SamplerFlags::UClamp) == wgfx::SamplerFlags::UClamp);
	CHECK((sampler & wgfx::SamplerFlags::MinPoint) == wgfx::SamplerFlags::MinPoint);

	CHECK(wgfx::border_color(3) == static_cast<wgfx::SamplerFlags>(3u << 24));

	const wgfx::DiscardFlags discard{
		wgfx::DiscardFlags::Bindings | wgfx::DiscardFlags::State };
	CHECK((discard & wgfx::DiscardFlags::Bindings)
		== wgfx::DiscardFlags::Bindings);
	CHECK((discard & wgfx::DiscardFlags::Transform)
		== wgfx::DiscardFlags::None);
}

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

	const u16 Indices[]{ 0, 1, 2 };

	wgfx::Texture color_texture()
	{
		wgfx::TextureDesc desc;
		desc.width = 4;
		desc.height = 4;
		desc.format = wgfx::TextureFormat::RGBA8;
		return wgfx::Texture::create(desc);
	}
} // anonymous namespace

TEST(view_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	CHECK(view.valid());
	CHECK(view.id() == 0);

	const wgfx::View invalid;
	CHECK(!invalid.valid());
	CHECK(invalid.id() == wgfx::View::InvalidId);

	CHECK(!wgfx::View::create(wgfx::View::InvalidId).valid());
	CHECK(!wgfx::View::create(
		static_cast<u16>(device->caps().maxViews)).valid());
}

TEST(view_is_copyable)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View first{ wgfx::View::create(0) };
	REQUIRE(first.valid());

	const wgfx::View second{ first };
	CHECK(first.valid());
	CHECK(second.valid());
	CHECK(second.id() == 0);
}

TEST(view_configures)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	CHECK(view.set_rect(0, 0, 320, 180));
	CHECK(view.set_rect(0, 0, wgfx::BackbufferRatio::Half));
	CHECK(view.set_scissor(0, 0, 160, 90));
	CHECK(view.set_clear(
		wgfx::ClearFlags::ColorDepth, 0x11223344, 1.0f, 0));
	CHECK(view.set_transform(IdentityMatrix, IdentityMatrix));
	CHECK(view.set_mode(wgfx::ViewMode::Sequential));
	CHECK(view.set_name("main"));
	CHECK(view.set_framebuffer());
	CHECK(view.touch());
	CHECK(view.reset_settings());

	wgfx::FrameBufferDesc desc;
	desc.width = 16;
	desc.height = 16;
	desc.colorFormat = wgfx::TextureFormat::RGBA8;
	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(desc) };
	REQUIRE(framebuffer.valid());
	CHECK(view.set_framebuffer(&framebuffer));
}

TEST(view_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	CHECK(!view.set_rect(0, 0, 0, 10));
	CHECK(!view.set_rect(0, 0, 10, 0));
	CHECK(!view.set_rect(0, 0, wgfx::BackbufferRatio::Explicit));
	CHECK(!view.set_transform(nullptr, IdentityMatrix));
	CHECK(!view.set_transform(IdentityMatrix, nullptr));
	CHECK(!view.set_mode(static_cast<wgfx::ViewMode>(wgfx::ViewMode::Count)));
	CHECK(!view.set_name(nullptr));
	CHECK(!view.set_clear(static_cast<wgfx::ClearFlags>(0x8000)));

	const wgfx::FrameBuffer invalidFramebuffer;
	CHECK(!view.set_framebuffer(&invalidFramebuffer));

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::DynamicVertexBuffer dynamicVertexBuffer{
		wgfx::DynamicVertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::DynamicIndexBuffer dynamicIndexBuffer{
		wgfx::DynamicIndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::Texture texture{ color_texture() };
	const wgfx::Uniform sampler{
		wgfx::Uniform::create("s_viewTest", wgfx::UniformType::Sampler) };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(dynamicVertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(dynamicIndexBuffer.valid());
	REQUIRE(texture.valid());
	REQUIRE(sampler.valid());

	CHECK(!view.set_vertex_buffer(4, vertexBuffer));
	CHECK(!view.set_vertex_buffer(4, dynamicVertexBuffer));
	CHECK(!view.set_vertex_buffer(0, wgfx::VertexBuffer{}));
	CHECK(!view.set_vertex_buffer(0, wgfx::DynamicVertexBuffer{}));
	CHECK(!view.set_vertex_buffer(0, vertexBuffer, 0, 0));
	CHECK(!view.set_vertex_buffer(0, dynamicVertexBuffer, 0, 0));
	CHECK(!view.set_index_buffer(wgfx::IndexBuffer{}));
	CHECK(!view.set_index_buffer(wgfx::DynamicIndexBuffer{}));
	CHECK(!view.set_index_buffer(indexBuffer, 0, 0));
	CHECK(!view.set_index_buffer(dynamicIndexBuffer, 0, 0));
	CHECK(!view.set_texture(16, sampler, texture));
	CHECK(!view.set_texture(0, wgfx::Uniform{}, texture));
	CHECK(!view.set_texture(0, sampler, wgfx::Texture{}));
	CHECK(!view.submit(wgfx::Program{}));

	const wgfx::View invalid;
	CHECK(!invalid.touch());
	CHECK(!invalid.reset_settings());
	CHECK(!invalid.discard());
	CHECK(!invalid.set_rect(0, 0, 16, 16));
}

TEST(view_binds_and_submits_draw)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());
	CHECK(view.set_rect(0, 0, 320, 180));
	CHECK(view.set_clear(wgfx::ClearFlags::ColorDepth));

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::Texture texture{ color_texture() };
	const wgfx::Uniform sampler{
		wgfx::Uniform::create("s_viewDraw", wgfx::UniformType::Sampler) };

	const wgfx::Shader vertex{
		wgfx::Shader::create(ViewVertexShader, sizeof(ViewVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ViewFragmentShader, sizeof(ViewFragmentShader)) };
	const wgfx::Program program{ wgfx::Program::create(vertex, fragment) };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(texture.valid());
	REQUIRE(sampler.valid());
	REQUIRE(program.valid());

	wgfx::RenderState state;
	state.flags = wgfx::RenderStateFlags::WriteRgb
		| wgfx::RenderStateFlags::WriteA
		| wgfx::RenderStateFlags::WriteZ
		| wgfx::RenderStateFlags::DepthTestLess
		| wgfx::RenderStateFlags::PrimitiveTriStrip;
	state.rgba = 0xffffffff;

	CHECK(view.set_state(state));
	CHECK(view.set_vertex_buffer(0, vertexBuffer));
	CHECK(view.set_index_buffer(indexBuffer, 0, 3));
	CHECK(view.set_texture(0, sampler, texture));
	CHECK(view.submit(program, 0));
	CHECK(view.touch());
	CHECK(view.discard());
}

TEST(view_binds_dynamic_buffers)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::DynamicVertexBuffer vertexBuffer{
		wgfx::DynamicVertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::DynamicIndexBuffer indexBuffer{
		wgfx::DynamicIndexBuffer::create(Indices, sizeof(Indices)) };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(indexBuffer.valid());

	CHECK(view.set_vertex_buffer(0, vertexBuffer, 0, 3));
	CHECK(view.set_index_buffer(indexBuffer, 0, 3));
	CHECK(view.set_state(wgfx::RenderStateFlags::WriteRgb, 0xffffffff));
}

TEST(view_submits_occlusion_query)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::OcclusionQuery query{ wgfx::OcclusionQuery::create() };
	const wgfx::Shader vertex{
		wgfx::Shader::create(ViewVertexShader, sizeof(ViewVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ViewFragmentShader, sizeof(ViewFragmentShader)) };
	const wgfx::Program program{ wgfx::Program::create(vertex, fragment) };

	REQUIRE(query.valid());
	REQUIRE(program.valid());
	CHECK(view.submit(program, query));
}

TEST(view_uses_instancing)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Triangle, sizeof(Triangle)) };
	const wgfx::DynamicVertexBuffer dynamicVertexBuffer{
		wgfx::DynamicVertexBuffer::create(layout, Triangle, sizeof(Triangle)) };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(dynamicVertexBuffer.valid());

	CHECK(view.set_instance_count(0));
	CHECK(view.set_instance_count(8));
	CHECK(view.set_instance_data_buffer(vertexBuffer, 0, 3));
	CHECK(view.set_instance_data_buffer(dynamicVertexBuffer, 1, 2));

	CHECK(!view.set_instance_data_buffer(wgfx::VertexBuffer{}, 0, 1));
	CHECK(!view.set_instance_data_buffer(wgfx::DynamicVertexBuffer{}, 0, 1));
	CHECK(!view.set_instance_data_buffer(vertexBuffer, 0, 0));
	CHECK(!view.set_instance_data_buffer(dynamicVertexBuffer, 0, 0));

	const wgfx::View invalid;
	CHECK(!invalid.set_instance_count(1));
	CHECK(!invalid.set_instance_data_buffer(vertexBuffer, 0, 1));
	CHECK(!invalid.set_instance_data_buffer(dynamicVertexBuffer, 0, 1));
}

TEST(view_submits_indirect_draw)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().drawIndirect);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::IndirectBuffer indirect{ wgfx::IndirectBuffer::create(2) };
	const wgfx::Shader vertex{
		wgfx::Shader::create(ViewVertexShader, sizeof(ViewVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ViewFragmentShader, sizeof(ViewFragmentShader)) };
	const wgfx::Program program{ wgfx::Program::create(vertex, fragment) };

	REQUIRE(indirect.valid());
	REQUIRE(program.valid());

	CHECK(view.submit(program, indirect));
	CHECK(view.submit(program, indirect, 1, 1));
	CHECK(view.submit(program, indirect, 0, 2, 3, wgfx::DiscardFlags::None));

	CHECK(!view.submit(program, indirect, 2, 1));
	CHECK(!view.submit(program, indirect, 0, 0));
	CHECK(!view.submit(program, indirect, 3, 0));
	CHECK(!view.submit(wgfx::Program{}, indirect));
	CHECK(!view.submit(program, wgfx::IndirectBuffer{}));

	const wgfx::View invalid;
	CHECK(!invalid.submit(program, indirect));
}

TEST(view_submits_indirect_count_draw)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().drawIndirect);
	REQUIRE(device->caps().drawIndirectCount);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	const wgfx::IndirectBuffer indirect{ wgfx::IndirectBuffer::create(2) };
	const u32 counts[]{ 1 };
	const wgfx::IndexBuffer numBuffer{
		wgfx::IndexBuffer::create(
			counts,
			sizeof(counts),
			wgfx::IndexType::Uint32,
			wgfx::BufferFlags::DrawIndirect) };
	const wgfx::IndexBuffer plainBuffer{
		wgfx::IndexBuffer::create(
			counts,
			sizeof(counts),
			wgfx::IndexType::Uint32) };
	const wgfx::IndexBuffer sixteenBit{
		wgfx::IndexBuffer::create(
			Indices,
			sizeof(Indices),
			wgfx::IndexType::Uint16,
			wgfx::BufferFlags::DrawIndirect) };

	const wgfx::Shader vertex{
		wgfx::Shader::create(ViewVertexShader, sizeof(ViewVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ViewFragmentShader, sizeof(ViewFragmentShader)) };
	const wgfx::Program program{ wgfx::Program::create(vertex, fragment) };

	REQUIRE(indirect.valid());
	REQUIRE(numBuffer.valid());
	REQUIRE(plainBuffer.valid());
	REQUIRE(sixteenBit.valid());
	REQUIRE(program.valid());

	CHECK_EQ(numBuffer.count(), 1u);
	CHECK(numBuffer.index_32());
	CHECK(numBuffer.has_flag(wgfx::BufferFlags::DrawIndirect));

	CHECK(view.submit(program, indirect, 0, numBuffer));
	CHECK(view.submit(program, indirect, 1, numBuffer, 0, 4));
	CHECK(view.submit(program, indirect, 0, numBuffer, 0, 4,
		2, wgfx::DiscardFlags::None));

	CHECK(!view.submit(program, indirect, 0, plainBuffer));
	CHECK(!view.submit(program, indirect, 0, sixteenBit));
	CHECK(!view.submit(program, indirect, 2, numBuffer));
	CHECK(!view.submit(program, indirect, 0, numBuffer, 1));
	CHECK(!view.submit(program, indirect, 0, numBuffer, 0, 0));
	CHECK(!view.submit(program, indirect, 0, wgfx::IndexBuffer{}));

	const wgfx::View invalid;
	CHECK(!invalid.submit(program, indirect, 0, numBuffer));
}

TEST(view_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	device->shutdown();
	CHECK(!view.valid());
	CHECK(view.id() == wgfx::View::InvalidId);
	CHECK(!view.set_rect(0, 0, 16, 16));
	CHECK(!view.touch());
	CHECK(!view.reset_settings());
	CHECK(!view.discard());

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!view.valid());

	const wgfx::View newView{ wgfx::View::create(0) };
	CHECK(newView.valid());
}

#else

TEST(view_requires_bgfx_backend)
{
	wgfx::View view{ wgfx::View::create(0) };
	CHECK(!view.valid());
	CHECK(view.id() == wgfx::View::InvalidId);

	CHECK(!view.set_rect(0, 0, 16, 16));
	CHECK(!view.set_rect(0, 0, wgfx::BackbufferRatio::Half));
	CHECK(!view.set_scissor());
	CHECK(!view.set_clear(wgfx::ClearFlags::ColorDepth));
	CHECK(!view.set_framebuffer());
	CHECK(!view.set_transform(IdentityMatrix, IdentityMatrix));
	CHECK(!view.set_mode(wgfx::ViewMode::Default));
	CHECK(!view.set_name("main"));
	CHECK(!view.reset_settings());
	CHECK(!view.touch());
	CHECK(!view.set_state(wgfx::RenderState{}));
	CHECK(!view.set_instance_count(1));
	CHECK(!view.set_instance_data_buffer(wgfx::VertexBuffer{}, 0, 1));
	CHECK(!view.set_instance_data_buffer(wgfx::DynamicVertexBuffer{}, 0, 1));
	CHECK(!view.submit(wgfx::Program{}, wgfx::IndirectBuffer{}));
	CHECK(!view.submit(
		wgfx::Program{}, wgfx::IndirectBuffer{}, 0, wgfx::IndexBuffer{}));
	CHECK(!view.discard());

	const wgfx::View copy{ view };
	CHECK(!copy.valid());
}

#endif