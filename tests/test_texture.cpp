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

	wgfx::TextureDesc rgba8_desc(u16 width, u16 height)
	{
		wgfx::TextureDesc desc;
		desc.width = width;
		desc.height = height;
		desc.format = wgfx::TextureFormat::RGBA8;
		return desc;
	}
} // anonymous namespace

TEST(texture_creates_immutable_from_data)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, pixels, sizeof(pixels)) };
	CHECK(texture.valid());
}

TEST(texture_creates_mipmapped_texture)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	// 4x4, 2x2 and 1x1 RGBA8 mip levels.
	const u8 pixels[4 * 4 * 4 + 2 * 2 * 4 + 1 * 1 * 4]{};
	wgfx::TextureDesc desc{ rgba8_desc(4, 4) };
	desc.mipmapped = true;

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, pixels, sizeof(pixels)) };
	CHECK(texture.valid());
}

TEST(texture_creates_array_texture)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 2 * 4]{};
	wgfx::TextureDesc desc{ rgba8_desc(2, 2) };
	desc.layers = 2;

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, pixels, sizeof(pixels)) };
	CHECK(texture.valid());
}

TEST(texture_creates_uninitialized_render_target)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::TextureDesc desc{ rgba8_desc(8, 8) };
	desc.flags = wgfx::TextureFlags::RenderTarget | wgfx::TextureFlags::Srgb;

	const wgfx::Texture texture{ wgfx::Texture::create(desc) };
	CHECK(texture.valid());
}

TEST(texture_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	wgfx::TextureDesc invalid{ desc };
	invalid.width = 0;
	CHECK(!wgfx::Texture::create(invalid, pixels, sizeof(pixels)).valid());

	invalid = desc;
	invalid.height = 0;
	CHECK(!wgfx::Texture::create(invalid, pixels, sizeof(pixels)).valid());

	invalid = desc;
	invalid.layers = 0;
	CHECK(!wgfx::Texture::create(invalid, pixels, sizeof(pixels)).valid());

	invalid = desc;
	invalid.format = static_cast<wgfx::TextureFormat>(wgfx::TextureFormat::Count);
	CHECK(!wgfx::Texture::create(invalid, pixels, sizeof(pixels)).valid());

	CHECK(!wgfx::Texture::create(desc, pixels, sizeof(pixels) - 1).valid());
	CHECK(!wgfx::Texture::create(desc, nullptr, sizeof(pixels)).valid());
	CHECK(!wgfx::Texture::create(desc, pixels, 0).valid());
}

TEST(texture_requires_active_device)
{
	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	CHECK(!wgfx::Texture::create(desc, pixels, sizeof(pixels)).valid());
	CHECK(!wgfx::Texture::create(desc).valid());
}

TEST(texture_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	auto first = wgfx::Texture::create(desc, pixels, sizeof(pixels));
	auto second = wgfx::Texture::create(desc);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::Texture moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(texture_instances_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	auto first = wgfx::Texture::create(desc, pixels, sizeof(pixels));
	auto second = wgfx::Texture::create(desc, pixels, sizeof(pixels));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(texture_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	auto texture = wgfx::Texture::create(desc, pixels, sizeof(pixels));
	REQUIRE(texture.valid());

	device->shutdown();
	CHECK(!texture.valid());
	CHECK(!wgfx::Texture::create(desc, pixels, sizeof(pixels)).valid());

	// The stale texture is left with an old generation; reset must be a no-op.
	texture.reset();
	CHECK(!texture.valid());
}

TEST(texture_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_desc(2, 2) };

	auto texture = wgfx::Texture::create(desc, pixels, sizeof(pixels));
	REQUIRE(texture.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!texture.valid());
}

#else

TEST(texture_requires_bgfx_backend)
{
	wgfx::TextureDesc desc;
	desc.width = 1;
	desc.height = 1;

	const u8 pixel[4]{};
	wgfx::Texture texture{
		wgfx::Texture::create(desc, pixel, sizeof(pixel)) };

	CHECK(!texture.valid());
	texture.reset();
}

#endif
