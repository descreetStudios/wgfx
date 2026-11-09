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

	wgfx::FrameBufferDesc color_desc(u16 width, u16 height)
	{
		wgfx::FrameBufferDesc desc;
		desc.width = width;
		desc.height = height;
		desc.colorFormat = wgfx::TextureFormat::RGBA8;
		return desc;
	}

	wgfx::Texture render_target(
		u16 width,
		u16 height,
		wgfx::TextureFormat format = wgfx::TextureFormat::RGBA8)
	{
		wgfx::TextureDesc desc;
		desc.width = width;
		desc.height = height;
		desc.format = format;
		desc.flags = wgfx::TextureFlags::RenderTarget;
		return wgfx::Texture::create(desc);
	}

	wgfx::FrameBufferAttachment attach(const wgfx::Texture& texture)
	{
		wgfx::FrameBufferAttachment attachment;
		attachment.texture = &texture;
		return attachment;
	}

	wgfx::SwapChainDesc swap_chain_desc(u32 width, u32 height)
	{
		wgfx::SwapChainDesc desc;
		desc.width = width;
		desc.height = height;
		desc.colorFormat = wgfx::TextureFormat::RGBA8;
		return desc;
	}
} // anonymous namespace

TEST(framebuffer_creates_color_target)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(color_desc(16, 16)) };
	CHECK(framebuffer.valid());
}

TEST(framebuffer_creates_color_depth_target)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::FrameBufferDesc desc{ color_desc(32, 16) };
	desc.depthFormat = wgfx::TextureFormat::D24S8;

	const wgfx::FrameBuffer framebuffer{ wgfx::FrameBuffer::create(desc) };
	CHECK(framebuffer.valid());
}

TEST(framebuffer_exposes_owned_textures)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::FrameBuffer color{
		wgfx::FrameBuffer::create(color_desc(16, 16)) };
	REQUIRE(color.valid());
	CHECK(color.color_texture().valid());
	CHECK(!color.depth_texture().valid());

	wgfx::FrameBufferDesc desc{ color_desc(32, 16) };
	desc.depthFormat = wgfx::TextureFormat::D24S8;

	const wgfx::FrameBuffer colorDepth{ 
		wgfx::FrameBuffer::create(desc) };
	REQUIRE(colorDepth.valid());
	CHECK(colorDepth.color_texture().valid());
	CHECK(colorDepth.depth_texture().valid());
}

TEST(framebuffer_does_not_expose_external_attachments)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto color = render_target(16, 16);
	REQUIRE(color.valid());

	const wgfx::FrameBufferAttachment attachment{ attach(color) };
	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(&attachment, 1) };
	REQUIRE(framebuffer.valid());
	CHECK(!framebuffer.color_texture().valid());
	CHECK(!framebuffer.depth_texture().valid());
}

TEST(framebuffer_moves_owned_textures)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::FrameBufferDesc desc{ color_desc(16, 16) };
	desc.depthFormat = wgfx::TextureFormat::D24S8;

	auto first = wgfx::FrameBuffer::create(desc);
	REQUIRE(first.valid());

	wgfx::FrameBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(!first.color_texture().valid());
	CHECK(!first.depth_texture().valid());
	CHECK(moved.color_texture().valid());
	CHECK(moved.depth_texture().valid());

	auto second = wgfx::FrameBuffer::create(color_desc(8, 8));
	REQUIRE(second.valid());
	second = std::move(moved);
	CHECK(second.color_texture().valid());
	CHECK(second.depth_texture().valid());
	CHECK(!moved.color_texture().valid());
	CHECK(!moved.depth_texture().valid());
}

TEST(framebuffer_reset_releases_owned_textures)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto framebuffer = wgfx::FrameBuffer::create(color_desc(16, 16));
	REQUIRE(framebuffer.valid());
	REQUIRE(framebuffer.color_texture().valid());

	framebuffer.reset();
	CHECK(!framebuffer.valid());
	CHECK(!framebuffer.color_texture().valid());
	CHECK(!framebuffer.depth_texture().valid());
}

TEST(framebuffer_owned_textures_die_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::FrameBufferDesc desc{ color_desc(16, 16) };
	desc.depthFormat = wgfx::TextureFormat::D24S8;

	auto framebuffer = wgfx::FrameBuffer::create(desc);
	REQUIRE(framebuffer.valid());
	REQUIRE(framebuffer.color_texture().valid());
	REQUIRE(framebuffer.depth_texture().valid());

	device->shutdown();
	CHECK(!framebuffer.valid());
	CHECK(!framebuffer.color_texture().valid());
	CHECK(!framebuffer.depth_texture().valid());
}

TEST(framebuffer_creates_from_backbuffer_ratio)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::FrameBufferDesc desc;
	desc.ratio = wgfx::BackbufferRatio::Half;
	desc.colorFormat = wgfx::TextureFormat::RGBA8;

	const wgfx::FrameBuffer framebuffer{ wgfx::FrameBuffer::create(desc) };
	REQUIRE(framebuffer.valid());
	CHECK(framebuffer.color_texture().valid());
	CHECK(!framebuffer.depth_texture().valid());
}

TEST(framebuffer_creates_ratio_with_depth)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::FrameBufferDesc desc;
	desc.ratio = wgfx::BackbufferRatio::Quarter;
	desc.colorFormat = wgfx::TextureFormat::RGBA8;
	desc.depthFormat = wgfx::TextureFormat::D24S8;

	const wgfx::FrameBuffer framebuffer{ wgfx::FrameBuffer::create(desc) };
	REQUIRE(framebuffer.valid());
	CHECK(framebuffer.color_texture().valid());
	CHECK(framebuffer.depth_texture().valid());
}

TEST(framebuffer_creates_from_attachments)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto color = render_target(16, 16);
	auto depth = render_target(16, 16, wgfx::TextureFormat::D24S8);
	REQUIRE(color.valid());
	REQUIRE(depth.valid());

	const wgfx::FrameBufferAttachment attachments[2]{
		attach(color),
		attach(depth),
	};

	wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(attachments, 2) };
	CHECK(framebuffer.valid());

	auto moved = std::move(framebuffer);
	moved.reset();
	CHECK(!moved.valid());
	CHECK(color.valid());
	CHECK(depth.valid());
}

TEST(framebuffer_creates_mrt)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = render_target(16, 16);
	auto second = render_target(16, 16);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	const wgfx::FrameBufferAttachment attachments[2]{
		attach(first),
		attach(second),
	};

	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(attachments, 2) };
	CHECK(framebuffer.valid());
}

TEST(framebuffer_supports_attachment_options)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto color = render_target(16, 16);
	REQUIRE(color.valid());

	wgfx::FrameBufferAttachment attachment{ attach(color) };
	attachment.access = wgfx::AttachmentAccess::ReadWrite;
	attachment.flags = wgfx::AttachmentFlags::None;

	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(&attachment, 1) };
	CHECK(framebuffer.valid());
}

TEST(framebuffer_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::FrameBufferDesc desc{ color_desc(16, 16) };

	desc.width = 0;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	desc = color_desc(16, 16);
	desc.height = 0;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	desc = color_desc(16, 16);
	desc.colorFormat = static_cast<wgfx::TextureFormat>(wgfx::TextureFormat::Count);
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	desc = color_desc(16, 16);
	desc.colorFormat = wgfx::TextureFormat::D24S8;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	desc = color_desc(16, 16);
	desc.depthFormat = wgfx::TextureFormat::RGBA8;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	// Attachments.
	CHECK(!wgfx::FrameBuffer::create(nullptr, 0).valid());
	CHECK(!wgfx::FrameBuffer::create(nullptr, 1).valid());

	wgfx::FrameBufferAttachment invalid;
	CHECK(!wgfx::FrameBuffer::create(&invalid, 1).valid());

	auto color = render_target(16, 16);
	auto depth = render_target(8, 8, wgfx::TextureFormat::D24S8);
	REQUIRE(color.valid());
	REQUIRE(depth.valid());

	const wgfx::FrameBufferAttachment mismatched[2]{
		attach(color),
		attach(depth),
	};
	CHECK(!wgfx::FrameBuffer::create(mismatched, 2).valid());

	wgfx::FrameBufferAttachment zeroLayers{ attach(color) };
	zeroLayers.layers = 0;
	CHECK(!wgfx::FrameBuffer::create(&zeroLayers, 1).valid());

	// A frame buffer attachment must be a render target.
	wgfx::TextureDesc plainDesc;
	plainDesc.width = 16;
	plainDesc.height = 16;
	plainDesc.format = wgfx::TextureFormat::RGBA8;
	auto plain = wgfx::Texture::create(plainDesc);
	REQUIRE(plain.valid());

	wgfx::FrameBufferAttachment plainAttachment{ attach(plain) };
	CHECK(!wgfx::FrameBuffer::create(&plainAttachment, 1).valid());

	// Too many attachments.
	wgfx::FrameBufferAttachment tooMany[9];
	for (wgfx::FrameBufferAttachment& attachment : tooMany)
		attachment = attach(color);
	CHECK(!wgfx::FrameBuffer::create(tooMany, 9).valid());
}

TEST(framebuffer_creates_swap_chain)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(swap_chain_desc(64, 48)) };
	REQUIRE(framebuffer.valid());

	// A swap chain cannot be sampled through the owned texture accessors.
	CHECK(!framebuffer.color_texture().valid());
	CHECK(!framebuffer.depth_texture().valid());
}

TEST(framebuffer_updates_swap_chain)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	auto framebuffer = wgfx::FrameBuffer::create(swap_chain_desc(64, 48));
	REQUIRE(framebuffer.valid());

	wgfx::SwapChainDesc desc{ swap_chain_desc(128, 96) };
	desc.flags = wgfx::SwapChainFlags::SrgbBackbuffer;
	CHECK(framebuffer.update_swap_chain(desc));
	CHECK(framebuffer.valid());
	CHECK_EQ(framebuffer.swap_chain_desc().width, 128u);
	CHECK_EQ(framebuffer.swap_chain_desc().height, 96u);
	CHECK(framebuffer.swap_chain_desc().flags
		== wgfx::SwapChainFlags::SrgbBackbuffer);

	// A regular render-target frame buffer is not a swap chain.
	auto regular = wgfx::FrameBuffer::create(color_desc(16, 16));
	REQUIRE(regular.valid());
	CHECK(!regular.update_swap_chain(swap_chain_desc(16, 16)));
}

TEST(framebuffer_exposes_swap_chain_description)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	wgfx::SwapChainDesc desc{ swap_chain_desc(64, 48) };
	desc.flags = wgfx::SwapChainFlags::SrgbBackbuffer;
	desc.numBackBuffers = 2;

	const wgfx::FrameBuffer framebuffer{ wgfx::FrameBuffer::create(desc) };
	REQUIRE(framebuffer.valid());
	CHECK(framebuffer.is_swap_chain());
	CHECK_EQ(framebuffer.swap_chain_desc().width, 64u);
	CHECK_EQ(framebuffer.swap_chain_desc().height, 48u);
	CHECK(framebuffer.swap_chain_desc().flags
		== wgfx::SwapChainFlags::SrgbBackbuffer);
	CHECK_EQ(framebuffer.swap_chain_desc().numBackBuffers, 2u);

	// Offscreen frame buffers do not report a swap chain description.
	const wgfx::FrameBuffer offscreen{
		wgfx::FrameBuffer::create(color_desc(16, 16)) };
	REQUIRE(offscreen.valid());
	CHECK(!offscreen.is_swap_chain());
	CHECK_EQ(offscreen.swap_chain_desc().width, 0u);
}

TEST(framebuffer_resizes_swap_chain_in_place)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	auto framebuffer = wgfx::FrameBuffer::create(swap_chain_desc(64, 48));
	REQUIRE(framebuffer.valid());

	CHECK(framebuffer.resize(128, 96));
	CHECK(framebuffer.valid());
	CHECK_EQ(framebuffer.swap_chain_desc().width, 128u);
	CHECK_EQ(framebuffer.swap_chain_desc().height, 96u);

	// The handle and other description fields survive the resize.
	CHECK(framebuffer.resize(256, 144));
	CHECK_EQ(framebuffer.swap_chain_desc().width, 256u);
	CHECK_EQ(framebuffer.swap_chain_desc().height, 144u);

	CHECK(!framebuffer.resize(0, 144));
	CHECK(!framebuffer.resize(256, 0));
	CHECK_EQ(framebuffer.swap_chain_desc().width, 256u);
	CHECK(framebuffer.valid());

	// A regular render-target frame buffer cannot be resized in place.
	auto regular = wgfx::FrameBuffer::create(color_desc(16, 16));
	REQUIRE(regular.valid());
	CHECK(!regular.resize(32, 32));
}

TEST(framebuffer_supports_multiple_windows)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	// Distinct native window handles. The Noop renderer never dereferences them.
	int firstWindow{ 0 };
	int secondWindow{ 0 };

	wgfx::SwapChainDesc firstDesc{ swap_chain_desc(64, 48) };
	firstDesc.window.handle = &firstWindow;
	wgfx::SwapChainDesc secondDesc{ swap_chain_desc(96, 72) };
	secondDesc.window.handle = &secondWindow;

	auto first = wgfx::FrameBuffer::create(firstDesc);
	auto second = wgfx::FrameBuffer::create(secondDesc);
	REQUIRE(first.valid());
	REQUIRE(second.valid());
	CHECK(first.is_swap_chain());
	CHECK(second.is_swap_chain());

	// Each window resizes independently.
	CHECK(first.resize(128, 96));
	CHECK_EQ(first.swap_chain_desc().width, 128u);
	CHECK_EQ(second.swap_chain_desc().width, 96u);
	CHECK_EQ(second.swap_chain_desc().height, 72u);

	CHECK(second.resize(192, 144));
	CHECK_EQ(second.swap_chain_desc().width, 192u);
	CHECK_EQ(first.swap_chain_desc().width, 128u);

	// Releasing one window leaves the other usable.
	second.reset();
	CHECK(!second.valid());
	CHECK(first.valid());
	CHECK(first.resize(320, 240));
}

TEST(framebuffer_rejects_invalid_swap_chain)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	CHECK(!wgfx::FrameBuffer::create(swap_chain_desc(0, 48)).valid());
	CHECK(!wgfx::FrameBuffer::create(swap_chain_desc(64, 0)).valid());

	wgfx::SwapChainDesc desc{ swap_chain_desc(64, 48) };
	desc.colorFormat = wgfx::TextureFormat::D24S8;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	desc = swap_chain_desc(64, 48);
	desc.depthFormat = wgfx::TextureFormat::RGBA8;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	auto framebuffer = wgfx::FrameBuffer::create(swap_chain_desc(64, 48));
	REQUIRE(framebuffer.valid());
	CHECK(!framebuffer.update_swap_chain(swap_chain_desc(0, 48)));
}

TEST(framebuffer_swap_chain_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	auto framebuffer = wgfx::FrameBuffer::create(swap_chain_desc(64, 48));
	REQUIRE(framebuffer.valid());
	REQUIRE(framebuffer.is_swap_chain());

	device->shutdown();
	CHECK(!framebuffer.valid());
	CHECK(!framebuffer.is_swap_chain());
	CHECK(!framebuffer.resize(128, 96));
	CHECK(!framebuffer.update_swap_chain(swap_chain_desc(64, 48)));
}

TEST(framebuffer_requires_active_device)
{
	wgfx::FrameBufferDesc desc{ color_desc(16, 16) };
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	auto color = render_target(16, 16);
	const wgfx::FrameBufferAttachment attachment{ attach(color) };
	CHECK(!wgfx::FrameBuffer::create(&attachment, 1).valid());

	const wgfx::FrameBuffer framebuffer;
	CHECK(!framebuffer.valid());
}

TEST(framebuffer_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::FrameBuffer::create(color_desc(16, 16));
	auto second = wgfx::FrameBuffer::create(color_desc(8, 8));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::FrameBuffer moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(framebuffer_instances_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::FrameBuffer::create(color_desc(16, 16));
	auto second = wgfx::FrameBuffer::create(color_desc(16, 16));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(framebuffer_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto framebuffer = wgfx::FrameBuffer::create(color_desc(16, 16));
	REQUIRE(framebuffer.valid());

	device->shutdown();
	CHECK(!framebuffer.valid());
	CHECK(!wgfx::FrameBuffer::create(color_desc(16, 16)).valid());

	// The stale frame buffer is left with an old generation; reset must be a no-op.
	framebuffer.reset();
	CHECK(!framebuffer.valid());
}

TEST(framebuffer_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto framebuffer = wgfx::FrameBuffer::create(color_desc(16, 16));
	REQUIRE(framebuffer.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!framebuffer.valid());
}

#else

TEST(framebuffer_requires_bgfx_backend)
{
	wgfx::FrameBufferDesc desc;
	desc.width = 16;
	desc.height = 16;

	wgfx::FrameBuffer framebuffer{ wgfx::FrameBuffer::create(desc) };
	CHECK(!framebuffer.valid());
	CHECK(!framebuffer.color_texture().valid());
	CHECK(!framebuffer.depth_texture().valid());

	desc = {};
	desc.ratio = wgfx::BackbufferRatio::Half;
	CHECK(!wgfx::FrameBuffer::create(desc).valid());

	const wgfx::FrameBufferAttachment attachment;
	CHECK(!wgfx::FrameBuffer::create(&attachment, 1).valid());

	wgfx::SwapChainDesc swapChain;
	swapChain.width = 16;
	swapChain.height = 16;
	wgfx::FrameBuffer swapChainBuffer{
		wgfx::FrameBuffer::create(swapChain) };
	CHECK(!swapChainBuffer.valid());
	CHECK(!swapChainBuffer.is_swap_chain());
	CHECK_EQ(swapChainBuffer.swap_chain_desc().width, 0u);
	CHECK(!swapChainBuffer.resize(16, 16));
	CHECK(!swapChainBuffer.update_swap_chain(swapChain));

	framebuffer.reset();
}

#endif
