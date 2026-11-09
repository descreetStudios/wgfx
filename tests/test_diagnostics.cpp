#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

using namespace wcore;
using namespace test;

namespace
{
	const u8 VertexShader[]
	{
		'V', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		1, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const float Vertices[]
	{
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};

	const u16 Indices[]{ 0, 1, 2 };
} // anonymous namespace

TEST(diagnostics_debug_flags_compose)
{
	const wgfx::DebugFlags flags{
		wgfx::DebugFlags::Stats | wgfx::DebugFlags::Text };
	CHECK((flags & wgfx::DebugFlags::Stats) == wgfx::DebugFlags::Stats);
	CHECK((flags & wgfx::DebugFlags::Text) == wgfx::DebugFlags::Text);
	CHECK((flags & wgfx::DebugFlags::Wireframe) == wgfx::DebugFlags::None);

	wgfx::DebugFlags mutableFlags{ wgfx::DebugFlags::None };
	mutableFlags |= wgfx::DebugFlags::Profiler;
	CHECK((mutableFlags & wgfx::DebugFlags::Profiler)
		== wgfx::DebugFlags::Profiler);

	CHECK_EQ(
		static_cast<u32>(wgfx::PrimitiveTopology::Count),
		5u);
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

	wgfx::Texture color_texture()
	{
		wgfx::TextureDesc desc;
		desc.width = 4;
		desc.height = 4;
		desc.format = wgfx::TextureFormat::RGBA8;
		return wgfx::Texture::create(desc);
	}
} // anonymous namespace

TEST(diagnostics_sets_debug_flags)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(device->set_debug(wgfx::DebugFlags::None));
	CHECK(device->set_debug(
		wgfx::DebugFlags::Stats | wgfx::DebugFlags::Text));
	CHECK(device->set_debug(wgfx::DebugFlags::Profiler));
	CHECK(!device->set_debug(
		static_cast<wgfx::DebugFlags>(0x80000000u)));

	wgfx::FrameBufferDesc desc;
	desc.width = 16;
	desc.height = 16;
	desc.colorFormat = wgfx::TextureFormat::RGBA8;
	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(desc) };
	REQUIRE(framebuffer.valid());
	CHECK(device->set_debug(wgfx::DebugFlags::Text, &framebuffer));

	const wgfx::FrameBuffer invalid;
	CHECK(!device->set_debug(wgfx::DebugFlags::Text, &invalid));

	device->shutdown();
	CHECK(!device->set_debug(wgfx::DebugFlags::Text));
}

TEST(diagnostics_writes_debug_text)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u16 image[]{ 0x1f41, 0x1f42 };

	CHECK(device->debug_text_clear());
	CHECK(device->debug_text_clear(0x1f, true));
	CHECK(device->debug_text_printf(0, 0, 0x0f, "wgfx %d", 1));
	CHECK(device->debug_text_image(0, 0, 2, 1, image, 4));

	CHECK(!device->debug_text_printf(0, 0, 0x0f, nullptr));
	CHECK(!device->debug_text_image(0, 0, 2, 1, nullptr, 4));

	CHECK(device->frame() > 0);

	device->shutdown();
	CHECK(!device->debug_text_clear());
	CHECK(!device->debug_text_printf(0, 0, 0x0f, "wgfx"));
	CHECK(!device->debug_text_image(0, 0, 2, 1, image, 4));
}

TEST(diagnostics_sets_markers)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(device->set_marker("wgfx.frame"));
	CHECK(device->set_marker("wgfx", 4));
	CHECK(!device->set_marker(nullptr));

	CHECK(device->frame() > 0);

	device->shutdown();
	CHECK(!device->set_marker("wgfx.frame"));
}

TEST(diagnostics_reports_stats)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Stats empty{ device->stats() };
	CHECK_EQ(static_cast<u32>(empty.width), device->width());
	CHECK_EQ(static_cast<u32>(empty.height), device->height());
	CHECK_EQ(
		static_cast<u32>(empty.numPrims[
			static_cast<u32>(wgfx::PrimitiveTopology::TriangleList)]),
		0u);

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Vertices, sizeof(Vertices)) };
	const wgfx::Texture texture{ color_texture() };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(texture.valid());

	const wgfx::Stats stats{ device->stats() };
	CHECK(stats.numVertexBuffers >= 1);
	CHECK(stats.numTextures >= 1);
	CHECK(stats.numVertexLayouts >= 1);

	const wgfx::Stats snapshot{ stats };
	CHECK_EQ(snapshot.numVertexBuffers, stats.numVertexBuffers);
	CHECK_EQ(snapshot.views.size(), stats.views.size());

	CHECK(device->frame() > 0);
	CHECK_EQ(snapshot.numVertexBuffers, stats.numVertexBuffers);
	CHECK_EQ(snapshot.views.size(), stats.views.size());
}

TEST(diagnostics_names_resources)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::VertexLayout layout{ position_layout() };
	const wgfx::VertexBuffer vertexBuffer{
		wgfx::VertexBuffer::create(layout, Vertices, sizeof(Vertices)) };
	const wgfx::IndexBuffer indexBuffer{
		wgfx::IndexBuffer::create(Indices, sizeof(Indices)) };
	const wgfx::Shader shader{
		wgfx::Shader::create(VertexShader, sizeof(VertexShader)) };
	const wgfx::Texture texture{ color_texture() };

	wgfx::FrameBufferDesc desc;
	desc.width = 16;
	desc.height = 16;
	desc.colorFormat = wgfx::TextureFormat::RGBA8;
	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(desc) };

	REQUIRE(vertexBuffer.valid());
	REQUIRE(indexBuffer.valid());
	REQUIRE(shader.valid());
	REQUIRE(texture.valid());
	REQUIRE(framebuffer.valid());

	CHECK(vertexBuffer.set_name("wgfx.vertex"));
	CHECK(indexBuffer.set_name("wgfx.index"));
	CHECK(shader.set_name("wgfx.shader", 11));
	CHECK(texture.set_name("wgfx.texture"));
	CHECK(framebuffer.set_name("wgfx.framebuffer"));

	CHECK(!vertexBuffer.set_name(nullptr));
	CHECK(!indexBuffer.set_name(nullptr));
	CHECK(!shader.set_name(nullptr));
	CHECK(!texture.set_name(nullptr));
	CHECK(!framebuffer.set_name(nullptr));

	const wgfx::VertexBuffer invalidVertexBuffer;
	const wgfx::IndexBuffer invalidIndexBuffer;
	const wgfx::Shader invalidShader;
	const wgfx::Texture invalidTexture;
	const wgfx::FrameBuffer invalidFrameBuffer;

	CHECK(!invalidVertexBuffer.set_name("wgfx.vertex"));
	CHECK(!invalidIndexBuffer.set_name("wgfx.index"));
	CHECK(!invalidShader.set_name("wgfx.shader"));
	CHECK(!invalidTexture.set_name("wgfx.texture"));
	CHECK(!invalidFrameBuffer.set_name("wgfx.framebuffer"));

	device->shutdown();
	CHECK(!vertexBuffer.set_name("wgfx.vertex"));
	CHECK(!indexBuffer.set_name("wgfx.index"));
	CHECK(!shader.set_name("wgfx.shader"));
	CHECK(!texture.set_name("wgfx.texture"));
	CHECK(!framebuffer.set_name("wgfx.framebuffer"));
}

#else

TEST(diagnostics_requires_bgfx_backend)
{
	const wgfx::VertexBuffer vertexBuffer;
	const wgfx::IndexBuffer indexBuffer;
	const wgfx::Shader shader;
	const wgfx::Texture texture;
	const wgfx::FrameBuffer framebuffer;

	CHECK(!vertexBuffer.set_name("wgfx.vertex"));
	CHECK(!indexBuffer.set_name("wgfx.index"));
	CHECK(!shader.set_name("wgfx.shader"));
	CHECK(!texture.set_name("wgfx.texture"));
	CHECK(!framebuffer.set_name("wgfx.framebuffer"));
}

#endif
