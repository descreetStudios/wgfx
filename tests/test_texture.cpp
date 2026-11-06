#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <utility>
#include <array>

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

	wgfx::TextureDesc rgba16_desc(u16 width, u16 height)
	{
		wgfx::TextureDesc desc{ rgba8_desc(width, height) };
		desc.format = wgfx::TextureFormat::RGBA16;
		return desc;
	}

	wgfx::TextureDesc rgba8_3d_desc(u16 width, u16 height, u16 depth)	{
		wgfx::TextureDesc desc{ rgba8_desc(width, height) };
		desc.type = wgfx::TextureType::Texture3D;
		desc.depth = depth;
		return desc;
	}

	wgfx::TextureDesc rgba8_cube_desc(u16 size, u16 layers = 1)
	{
		wgfx::TextureDesc desc{ rgba8_desc(size, size) };
		desc.type = wgfx::TextureType::TextureCube;
		desc.layers = layers;
		return desc;
	}

	// Minimal 2x2 RGBA8 KTX 1.1 container with a single mip level.
	std::array<u8, 84> minimal_ktx()
	{
		return {
			0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB,
			0x0D, 0x0A, 0x1A, 0x0A, // identifier
			0x01, 0x02, 0x03, 0x04, // endianness
			0x01, 0x14, 0x00, 0x00, // glType = UNSIGNED_BYTE
			0x01, 0x00, 0x00, 0x00, // glTypeSize
			0x08, 0x19, 0x00, 0x00, // glFormat = RGBA
			0x58, 0x80, 0x00, 0x00, // glInternalFormat = RGBA8
			0x08, 0x19, 0x00, 0x00, // glBaseInternalFormat = RGBA
			0x02, 0x00, 0x00, 0x00, // width
			0x02, 0x00, 0x00, 0x00, // height
			0x00, 0x00, 0x00, 0x00, // depth
			0x00, 0x00, 0x00, 0x00, // array elements
			0x01, 0x00, 0x00, 0x00, // faces
			0x01, 0x00, 0x00, 0x00, // mip levels
			0x00, 0x00, 0x00, 0x00, // key/value bytes
			0x10, 0x00, 0x00, 0x00, // image size
			0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
			0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		};
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

TEST(texture_creates_3d_from_data)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 voxels[2 * 2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_3d_desc(2, 2, 2) };

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, voxels, sizeof(voxels)) };
	CHECK(texture.valid());
}

TEST(texture_creates_uninitialized_3d)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Texture texture{
		wgfx::Texture::create(rgba8_3d_desc(4, 4, 4)) };
	CHECK(texture.valid());
}

TEST(texture_creates_mipmapped_3d)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	// 4x4x4, 2x2x2 and 1x1x1 RGBA8 mip levels.
	const u8 voxels[4 * 4 * 4 * 4 + 2 * 2 * 2 * 4 + 1 * 1 * 1 * 4]{};
	wgfx::TextureDesc desc{ rgba8_3d_desc(4, 4, 4) };
	desc.mipmapped = true;

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, voxels, sizeof(voxels)) };
	CHECK(texture.valid());
}

TEST(texture_rejects_invalid_3d_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 voxels[2 * 2 * 2 * 4]{};

	wgfx::TextureDesc invalid{ rgba8_3d_desc(2, 2, 2) };
	invalid.depth = 0;
	CHECK(!wgfx::Texture::create(invalid).valid());

	invalid = rgba8_3d_desc(2, 2, 2);
	invalid.layers = 2;
	CHECK(!wgfx::Texture::create(invalid).valid());

	invalid = rgba8_3d_desc(2, 2, 2);
	invalid.type = static_cast<wgfx::TextureType>(3);
	CHECK(!wgfx::Texture::create(invalid).valid());

	invalid = rgba8_desc(2, 2);
	invalid.depth = 2;
	CHECK(!wgfx::Texture::create(invalid).valid());

	CHECK(!wgfx::Texture::create(
		rgba8_3d_desc(2, 2, 2),
		voxels,
		sizeof(voxels) - 1).valid());
}

TEST(texture_creates_cube_from_data)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[6 * 2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_cube_desc(2) };

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, pixels, sizeof(pixels)) };
	CHECK(texture.valid());
}

TEST(texture_creates_uninitialized_cube)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Texture texture{
		wgfx::Texture::create(rgba8_cube_desc(4)) };
	CHECK(texture.valid());
}

TEST(texture_creates_cube_array)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 6 * 2 * 2 * 4]{};
	const wgfx::TextureDesc desc{ rgba8_cube_desc(2, 2) };

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, pixels, sizeof(pixels)) };
	CHECK(texture.valid());
}

TEST(texture_creates_mipmapped_cube)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	// Six 4x4, 2x2 and 1x1 RGBA8 faces.
	const u8 pixels[6 * (4 * 4 + 2 * 2 + 1 * 1) * 4]{};
	wgfx::TextureDesc desc{ rgba8_cube_desc(4) };
	desc.mipmapped = true;

	const wgfx::Texture texture{
		wgfx::Texture::create(desc, pixels, sizeof(pixels)) };
	CHECK(texture.valid());
}

TEST(texture_rejects_invalid_cube_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[6 * 2 * 2 * 4]{};

	wgfx::TextureDesc invalid{ rgba8_cube_desc(2) };
	invalid.height = 3;
	CHECK(!wgfx::Texture::create(invalid).valid());

	invalid = rgba8_cube_desc(2);
	invalid.depth = 2;
	CHECK(!wgfx::Texture::create(invalid).valid());

	CHECK(!wgfx::Texture::create(
		rgba8_cube_desc(2),
		pixels,
		sizeof(pixels) - 1).valid());
}

TEST(texture_creates_from_backbuffer_ratio)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::TextureDesc desc;
	desc.ratio = wgfx::BackbufferRatio::Half;
	desc.format = wgfx::TextureFormat::RGBA8;
	CHECK(wgfx::Texture::create(desc).valid());

	desc = {};
	desc.ratio = wgfx::BackbufferRatio::Double;
	desc.layers = 2;
	desc.mipmapped = true;
	CHECK(wgfx::Texture::create(desc).valid());
}

TEST(texture_rejects_invalid_ratio_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixel[4]{};
	wgfx::TextureDesc desc;
	desc.ratio = wgfx::BackbufferRatio::Half;

	CHECK(!wgfx::Texture::create(
		desc, pixel, sizeof(pixel)).valid());

	desc.type = wgfx::TextureType::Texture3D;
	desc.depth = 2;
	CHECK(!wgfx::Texture::create(desc).valid());

	desc = {};
	desc.ratio = wgfx::BackbufferRatio::Half;
	desc.type = wgfx::TextureType::TextureCube;
	CHECK(!wgfx::Texture::create(desc).valid());
}

TEST(texture_creates_from_container)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto ktx = minimal_ktx();
	const wgfx::Texture texture{
		wgfx::Texture::create(
			ktx.data(),
			static_cast<u32>(ktx.size())) };
	CHECK(texture.valid());
}

TEST(texture_creates_from_container_with_flags)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto ktx = minimal_ktx();
	const wgfx::Texture texture{
		wgfx::Texture::create(
			ktx.data(),
			static_cast<u32>(ktx.size()),
			wgfx::TextureFlags::Srgb) };
	CHECK(texture.valid());
}

TEST(texture_rejects_invalid_container)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto ktx = minimal_ktx();

	CHECK(!wgfx::Texture::create(nullptr, 0).valid());
	CHECK(!wgfx::Texture::create(ktx.data(), 0).valid());

	const u8 garbage[16]{};
	CHECK(!wgfx::Texture::create(garbage, sizeof(garbage)).valid());

	CHECK(!wgfx::Texture::create(ktx.data(), 20).valid());
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

TEST(texture_updates_2d)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto texture = wgfx::Texture::create(rgba8_desc(4, 4));
	REQUIRE(texture.valid());

	const u8 pixels[2 * 2 * 4]{};
	CHECK(texture.update2D(0, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));
	CHECK(texture.update2D(0, 0, 1, 1, 2, 2, pixels, sizeof(pixels)));

	// Out of bounds, empty data, and layer on a non-array texture.
	CHECK(!texture.update2D(0, 0, 3, 0, 2, 2, pixels, sizeof(pixels)));
	CHECK(!texture.update2D(0, 0, 0, 3, 2, 2, pixels, sizeof(pixels)));
	CHECK(!texture.update2D(0, 0, 0, 0, 2, 2, nullptr, 0));
	CHECK(!texture.update2D(0, 0, 0, 0, 2, 2, pixels, 0));
	CHECK(!texture.update2D(1, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));
}

TEST(texture_update_2d_respects_mips_and_immutability)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::TextureDesc desc{ rgba8_desc(4, 4) };
	desc.mipmapped = true;
	auto texture = wgfx::Texture::create(desc);
	REQUIRE(texture.valid());

	const u8 pixel[4]{};
	const u8 mipPixels[4 * 4 * 4]{};
	CHECK(texture.update2D(0, 0, 0, 0, 4, 4, mipPixels, sizeof(mipPixels)));
	CHECK(texture.update2D(0, 2, 0, 0, 1, 1, pixel, sizeof(pixel)));
	CHECK(!texture.update2D(0, 3, 0, 0, 1, 1, pixel, sizeof(pixel)));

	wgfx::TextureDesc immutableDesc{ rgba8_desc(2, 2) };
	const u8 pixels[2 * 2 * 4]{};
	auto immutable = wgfx::Texture::create(immutableDesc, pixels, sizeof(pixels));
	REQUIRE(immutable.valid());
	CHECK(!immutable.update2D(0, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));
}

TEST(texture_updates_3d)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto texture = wgfx::Texture::create(rgba8_3d_desc(4, 4, 4));
	REQUIRE(texture.valid());

	const u8 voxels[2 * 2 * 2 * 4]{};
	CHECK(texture.update3D(0, 0, 0, 0, 2, 2, 2, voxels, sizeof(voxels)));
	CHECK(texture.update3D(0, 1, 1, 1, 2, 2, 2, voxels, sizeof(voxels)));

	CHECK(!texture.update3D(0, 3, 0, 0, 2, 2, 2, voxels, sizeof(voxels)));
	CHECK(!texture.update3D(0, 0, 0, 0, 2, 2, 0, voxels, sizeof(voxels)));
	CHECK(!texture.update3D(0, 0, 0, 0, 2, 2, 2, nullptr, 0));

	auto flat = wgfx::Texture::create(rgba8_desc(4, 4));
	REQUIRE(flat.valid());
	CHECK(!flat.update3D(0, 0, 0, 0, 2, 2, 1, voxels, sizeof(voxels)));
}

TEST(texture_updates_cube)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto texture = wgfx::Texture::create(rgba8_cube_desc(4));
	REQUIRE(texture.valid());

	const u8 pixels[2 * 2 * 4]{};
	CHECK(texture.updateCube(0, 0, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));
	CHECK(texture.updateCube(0, 5, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));

	CHECK(!texture.updateCube(0, 6, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));

	auto array = wgfx::Texture::create(rgba8_cube_desc(4, 2));
	REQUIRE(array.valid());
	CHECK(array.updateCube(1, 0, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));
	CHECK(!array.updateCube(2, 0, 0, 0, 0, 2, 2, pixels, sizeof(pixels)));
}

TEST(texture_read_requires_read_back_flag)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[2 * 2 * 4]{};
	wgfx::TextureDesc desc{ rgba8_desc(2, 2) };
	desc.flags = wgfx::TextureFlags::ReadBack;

	auto readBack = wgfx::Texture::create(desc, pixels, sizeof(pixels));
	REQUIRE(readBack.valid());

	u8 result[2 * 2 * 4]{};
	CHECK(readBack.read(result) > 0);
	CHECK(readBack.read(nullptr) == 0);

	auto plain = wgfx::Texture::create(rgba8_desc(2, 2));
	REQUIRE(plain.valid());
	CHECK(plain.read(result) == 0);

	wgfx::Texture invalid;
	CHECK(invalid.read(result) == 0);
}

TEST(texture_blits_between_textures)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u8 pixels[4 * 4 * 4]{};
	const auto src = wgfx::Texture::create(rgba8_desc(4, 4), pixels, sizeof(pixels));
	REQUIRE(src.valid());

	wgfx::TextureDesc desc{ rgba8_desc(4, 4) };
	desc.flags = wgfx::TextureFlags::BlitDestination;
	const auto dst = wgfx::Texture::create(desc);
	REQUIRE(dst.valid());

	wgfx::TextureRegion dstRegion{};
	dstRegion.texture = &dst;
	dstRegion.width = 2;
	dstRegion.height = 2;

	wgfx::TextureRegion srcRegion{};
	srcRegion.texture = &src;
	srcRegion.width = 2;
	srcRegion.height = 2;

	CHECK(wgfx::Texture::blit(0, dstRegion, srcRegion));

	// Missing destination flag, mismatched formats, and identical textures.
	wgfx::TextureRegion badDst{ srcRegion };
	badDst.texture = &src;
	CHECK(!wgfx::Texture::blit(0, badDst, srcRegion));

	auto otherFormat = wgfx::Texture::create(rgba16_desc(2, 2));
	REQUIRE(otherFormat.valid());
	wgfx::TextureRegion otherRegion{};
	otherRegion.texture = &otherFormat;
	CHECK(!wgfx::Texture::blit(0, otherRegion, srcRegion));

	wgfx::TextureRegion missing{};
	CHECK(!wgfx::Texture::blit(0, missing, srcRegion));

	CHECK(!wgfx::Texture::blit(0, dstRegion, dstRegion));
}

TEST(texture_requires_active_device)
{
	const auto ktx = minimal_ktx();
	CHECK(!wgfx::Texture::create(
		ktx.data(),
		static_cast<u32>(ktx.size())).valid());
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

	wgfx::TextureDesc desc3D;
	desc3D.width = 1;
	desc3D.height = 1;
	desc3D.depth = 1;
	desc3D.type = wgfx::TextureType::Texture3D;
	wgfx::Texture texture3D{ wgfx::Texture::create(desc3D) };

	wgfx::TextureDesc descCube;
	descCube.width = 1;
	descCube.height = 1;
	descCube.type = wgfx::TextureType::TextureCube;
	wgfx::Texture textureCube{ wgfx::Texture::create(descCube) };

	const u8 container[16]{};
	wgfx::Texture containerTexture{
		wgfx::Texture::create(container, sizeof(container)) };

	CHECK(!texture.valid());
	CHECK(!texture3D.valid());
	CHECK(!textureCube.valid());
	CHECK(!containerTexture.valid());

	const u8 pixels[4 * 4 * 4]{};
	CHECK(!texture.update2D(0, 0, 0, 0, 1, 1, pixels, sizeof(pixels)));
	CHECK(!texture3D.update3D(0, 0, 0, 0, 1, 1, 1, pixels, sizeof(pixels)));
	CHECK(!textureCube.updateCube(0, 0, 0, 0, 0, 1, 1, pixels, sizeof(pixels)));

	u8 result[4 * 4 * 4]{};
	CHECK(texture.read(result) == 0);

	const wgfx::TextureRegion region{};
	CHECK(!wgfx::Texture::blit(0, region, region));

	texture.reset();
	texture3D.reset();
	textureCube.reset();
	containerTexture.reset();
}

#endif
