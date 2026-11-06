#include <wgfx/texture.hpp>

#include "device_internal.hpp"

#include <cstring>

#if defined(WGFX_BACKEND_BGFX)
#include "texture_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
	namespace
	{
		u32 mip_extent(u32 base, u8 mip)
		{
			if (base == 0)
				return 0;

			const u32 extent{ base >> mip };
			return extent == 0 ? 1 : extent;
		}

		bool region_within(u32 extent, u16 origin, u16 size)
		{
			if (origin >= extent)
				return false;

			return size == 0
				|| static_cast<u32>(origin) + size <= extent;
		}

		u8 calc_num_mips(bool mipmapped, u16 width, u16 height, u16 depth)
		{
			if (!mipmapped)
				return 1;

			u32 largest{ width > height ? width : height };
			if (depth > largest)
				largest = depth;

			u8 count{ 1 };
			while (largest > 1)
			{
				largest >>= 1;
				++count;
			}

			return count;
		}
	} // anonymous namespace

#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		static_assert(static_cast<u16>(TextureFormat::Count) == bgfx::TextureFormat::Count,
			"TextureFormat must mirror bgfx::TextureFormat");
		static_assert(static_cast<u16>(TextureFormat::BC1) == bgfx::TextureFormat::BC1,
			"TextureFormat::BC1 must mirror bgfx::TextureFormat::BC1");
		static_assert(static_cast<u16>(TextureFormat::RGBA8) == bgfx::TextureFormat::RGBA8,
			"TextureFormat::RGBA8 must mirror bgfx::TextureFormat::RGBA8");
		static_assert(static_cast<u16>(TextureFormat::D24S8) == bgfx::TextureFormat::D24S8,
			"TextureFormat::D24S8 must mirror bgfx::TextureFormat::D24S8");
		static_assert(static_cast<u16>(TextureFormat::D0S8) == bgfx::TextureFormat::D0S8,
			"TextureFormat::D0S8 must mirror bgfx::TextureFormat::D0S8");

		static_assert(static_cast<u8>(BackbufferRatio::Equal) == bgfx::BackbufferRatio::Equal,
			"BackbufferRatio::Equal must mirror bgfx::BackbufferRatio::Equal");
		static_assert(static_cast<u8>(BackbufferRatio::Half) == bgfx::BackbufferRatio::Half,
			"BackbufferRatio::Half must mirror bgfx::BackbufferRatio::Half");
		static_assert(static_cast<u8>(BackbufferRatio::Quarter) == bgfx::BackbufferRatio::Quarter,
			"BackbufferRatio::Quarter must mirror bgfx::BackbufferRatio::Quarter");
		static_assert(static_cast<u8>(BackbufferRatio::Eighth) == bgfx::BackbufferRatio::Eighth,
			"BackbufferRatio::Eighth must mirror bgfx::BackbufferRatio::Eighth");
		static_assert(static_cast<u8>(BackbufferRatio::Sixteenth) == bgfx::BackbufferRatio::Sixteenth,
			"BackbufferRatio::Sixteenth must mirror bgfx::BackbufferRatio::Sixteenth");
		static_assert(static_cast<u8>(BackbufferRatio::Double) == bgfx::BackbufferRatio::Double,
			"BackbufferRatio::Double must mirror bgfx::BackbufferRatio::Double");
		static_assert(static_cast<u8>(BackbufferRatio::Explicit) == bgfx::BackbufferRatio::Count,
			"BackbufferRatio::Explicit must map to bgfx::BackbufferRatio::Count");

		static_assert(static_cast<u64>(TextureFlags::None) == BGFX_TEXTURE_NONE,
			"TextureFlags::None must mirror BGFX_TEXTURE_NONE");
		static_assert(static_cast<u64>(TextureFlags::MsaaSample) == BGFX_TEXTURE_MSAA_SAMPLE,
			"TextureFlags::MsaaSample must mirror BGFX_TEXTURE_MSAA_SAMPLE");
		static_assert(static_cast<u64>(TextureFlags::RenderTarget) == BGFX_TEXTURE_RT,
			"TextureFlags::RenderTarget must mirror BGFX_TEXTURE_RT");
		static_assert(static_cast<u64>(TextureFlags::RenderTargetMsaaX2) == BGFX_TEXTURE_RT_MSAA_X2,
			"TextureFlags::RenderTargetMsaaX2 must mirror BGFX_TEXTURE_RT_MSAA_X2");
		static_assert(static_cast<u64>(TextureFlags::RenderTargetMsaaX4) == BGFX_TEXTURE_RT_MSAA_X4,
			"TextureFlags::RenderTargetMsaaX4 must mirror BGFX_TEXTURE_RT_MSAA_X4");
		static_assert(static_cast<u64>(TextureFlags::RenderTargetMsaaX8) == BGFX_TEXTURE_RT_MSAA_X8,
			"TextureFlags::RenderTargetMsaaX8 must mirror BGFX_TEXTURE_RT_MSAA_X8");
		static_assert(static_cast<u64>(TextureFlags::RenderTargetMsaaX16) == BGFX_TEXTURE_RT_MSAA_X16,
			"TextureFlags::RenderTargetMsaaX16 must mirror BGFX_TEXTURE_RT_MSAA_X16");
		static_assert(static_cast<u64>(TextureFlags::RenderTargetWriteOnly) == BGFX_TEXTURE_RT_WRITE_ONLY,
			"TextureFlags::RenderTargetWriteOnly must mirror BGFX_TEXTURE_RT_WRITE_ONLY");
		static_assert(static_cast<u64>(TextureFlags::ComputeWrite) == BGFX_TEXTURE_COMPUTE_WRITE,
			"TextureFlags::ComputeWrite must mirror BGFX_TEXTURE_COMPUTE_WRITE");
		static_assert(static_cast<u64>(TextureFlags::Srgb) == BGFX_TEXTURE_SRGB,
			"TextureFlags::Srgb must mirror BGFX_TEXTURE_SRGB");
		static_assert(static_cast<u64>(TextureFlags::BlitDestination) == BGFX_TEXTURE_BLIT_DST,
			"TextureFlags::BlitDestination must mirror BGFX_TEXTURE_BLIT_DST");
		static_assert(static_cast<u64>(TextureFlags::ReadBack) == BGFX_TEXTURE_READ_BACK,
			"TextureFlags::ReadBack must mirror BGFX_TEXTURE_READ_BACK");
		static_assert(static_cast<u64>(TextureFlags::ExternalShared) == BGFX_TEXTURE_EXTERNAL_SHARED,
			"TextureFlags::ExternalShared must mirror BGFX_TEXTURE_EXTERNAL_SHARED");
		static_assert(static_cast<u64>(TextureFlags::SrgbMutable) == BGFX_TEXTURE_SRGB_MUTABLE,
			"TextureFlags::SrgbMutable must mirror BGFX_TEXTURE_SRGB_MUTABLE");

		bgfx::TextureFormat::Enum to_bgfx(TextureFormat format)
		{
			return static_cast<bgfx::TextureFormat::Enum>(format);
		}

		bgfx::BackbufferRatio::Enum to_bgfx(BackbufferRatio ratio)
		{
			return ratio == BackbufferRatio::Explicit
				? bgfx::BackbufferRatio::Count
				: static_cast<bgfx::BackbufferRatio::Enum>(ratio);
		}

		bgfx::TextureRegion to_bgfx(const TextureRegion& region)
		{
			return bgfx::TextureRegion{
				detail::TextureAccess::native(*region.texture),
				region.mip,
				region.x,
				region.y,
				region.z,
				region.width,
				region.height,
				region.depth,
			};
		}

		bool is_image_container(const void* data, u32 size)
		{
			if (!data)
				return false;

			static constexpr u8 dds[4]{ 'D', 'D', 'S', ' ' };
			static constexpr u8 ktx[4]{ 0xAB, 'K', 'T', 'X' };
			static constexpr u8 pvr3[4]{ 'P', 'V', 'R', 3 };

			const u8* bytes{ static_cast<const u8*>(data) };
			return (size >= 128 && std::memcmp(bytes, dds, sizeof(dds)) == 0)
				|| (size >= 64 && std::memcmp(bytes, ktx, sizeof(ktx)) == 0)
				|| (size >= 52 && std::memcmp(bytes, pvr3, sizeof(pvr3)) == 0);
		}
	} // anonymous namespace
#endif

	Texture::Texture(Texture&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mDesc{ other.mDesc }
		, mNumMips{ other.mNumMips }
		, mImmutable{ other.mImmutable }
		, mDescKnown{ other.mDescKnown }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mDesc = {};
		other.mNumMips = 0;
		other.mImmutable = false;
		other.mDescKnown = false;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mDesc = other.mDesc;
		mNumMips = other.mNumMips;
		mImmutable = other.mImmutable;
		mDescKnown = other.mDescKnown;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mDesc = {};
		other.mNumMips = 0;
		other.mImmutable = false;
		other.mDescKnown = false;
		return *this;
	}

	Texture::~Texture()
	{
		reset();
	}

	Texture Texture::create(
		const TextureDesc& desc,
		const void* data,
		u32 size)
	{
		Texture result;

#if defined(WGFX_BACKEND_BGFX)
		const u16 formatIndex{ static_cast<u16>(desc.format) };
		if (desc.layers == 0
		|| formatIndex >= static_cast<u16>(TextureFormat::Count))
		{
			return result;
		}

		const bool ratioBased{ desc.ratio != BackbufferRatio::Explicit };
		const bool is3D{ desc.type == TextureType::Texture3D };
		const bool isCube{ desc.type == TextureType::TextureCube };
		if (desc.type != TextureType::Texture2D && !is3D && !isCube)
			return result;

		if (ratioBased)
		{
			if (is3D || isCube || desc.depth != 1
			|| data != nullptr || size != 0)
			{
				return result;
			}
		}
		else
		{
			if (desc.width == 0 || desc.height == 0)
				return result;

			if (is3D)
			{
				if (desc.depth == 0 || desc.layers != 1)
					return result;
			}
			else if (isCube)
			{
				if (desc.depth != 1 || desc.width != desc.height)
					return result;
			}
			else if (desc.depth != 1)
			{
				return result;
			}

			if ((data == nullptr) != (size == 0))
				return result;
		}

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::TextureFormat::Enum format{ to_bgfx(desc.format) };
		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };

		if (ratioBased)
		{
			handle = bgfx::createTexture2D(
				to_bgfx(desc.ratio),
				desc.mipmapped,
				desc.layers,
				format,
				static_cast<u64>(desc.flags));
		}
		else
		{
			const u16 depth{ is3D ? desc.depth : u16{ 1 } };
			const u16 numLayers{ is3D ? u16{ 1 } : desc.layers };

			bgfx::TextureInfo info{};
			bgfx::calcTextureSize(
				info,
				desc.width,
				desc.height,
				depth,
				isCube,
				desc.mipmapped,
				numLayers,
				format);

			const bgfx::Memory* memory{ nullptr };
			if (data)
			{
				if (size != info.storageSize)
					return result;

				memory = bgfx::copy(data, size);
				if (!memory)
					return result;
			}

			if (is3D)
			{
				handle = bgfx::createTexture3D(
					desc.width,
					desc.height,
					desc.depth,
					desc.mipmapped,
					format,
					static_cast<u64>(desc.flags),
					memory);
			}
			else if (isCube)
			{
				handle = bgfx::createTextureCube(
					desc.width,
					desc.mipmapped,
					desc.layers,
					format,
					static_cast<u64>(desc.flags),
					memory);
			}
			else
			{
				handle = bgfx::createTexture2D(
					desc.width,
					desc.height,
					desc.mipmapped,
					desc.layers,
					format,
					static_cast<u64>(desc.flags),
					memory);
			}
		}

		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mDesc = desc;
		result.mNumMips = ratioBased
			? 0
			: calc_num_mips(
				desc.mipmapped,
				desc.width,
				desc.height,
				is3D ? desc.depth : u16{ 1 });
		result.mImmutable = data != nullptr;
		result.mDescKnown = true;
#else
		(void)desc;
		(void)data;
		(void)size;
#endif

		return result;
	}

	Texture Texture::create(
		const void* data,
		u32 size,
		TextureFlags flags,
		u8 skip)
	{
		Texture result;

#if defined(WGFX_BACKEND_BGFX)
		if (!data || size == 0 || !is_image_container(data, size))
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return result;

		const bgfx::TextureHandle handle{ bgfx::createTexture(
			memory,
			static_cast<u64>(flags),
			skip) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mDesc = {};
		result.mDesc.flags = flags;
		result.mImmutable = true;
#else
		(void)data;
		(void)size;
		(void)flags;
		(void)skip;
#endif

		return result;
	}

	bool Texture::mip_valid(u8 mip) const
	{
		return mNumMips == 0 || mip < mNumMips;
	}

	bool Texture::region_valid(
		u8 mip,
		u16 x,
		u16 y,
		u16 z,
		u16 width,
		u16 height,
		u16 depth) const
	{
		if (!mip_valid(mip))
			return false;

		// Ratio-based and container textures do not expose their dimensions.
		if (mDesc.width == 0 || mDesc.height == 0)
			return true;

		const u32 mipWidth{ mip_extent(mDesc.width, mip) };
		const u32 mipHeight{ mip_extent(mDesc.height, mip) };

		u32 mipDepth{ 1 };
		switch (mDesc.type)
		{
		case TextureType::Texture3D:
			mipDepth = mip_extent(mDesc.depth, mip);
			break;
		case TextureType::TextureCube:
			mipDepth = 6u * mDesc.layers;
			break;
		case TextureType::Texture2D:
			mipDepth = mDesc.layers;
			break;
		}

		return region_within(mipWidth, x, width)
			&& region_within(mipHeight, y, height)
			&& region_within(mipDepth, z, depth);
	}

	bool Texture::update2D(
		u16 layer,
		u8 mip,
		u16 x,
		u16 y,
		u16 width,
		u16 height,
		const void* data,
		u32 size,
		u16 pitch)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| mImmutable
		|| !data
		|| size == 0
		|| width == 0
		|| height == 0)
		{
			return false;
		}

		if (mDescKnown && mDesc.type != TextureType::Texture2D)
			return false;

		if (mDescKnown && layer >= mDesc.layers)
			return false;

		if (!region_valid(mip, x, y, layer, width, height, 1))
			return false;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return false;

		bgfx::updateTexture2D(
			detail::TextureAccess::native(*this),
			layer,
			mip,
			x,
			y,
			width,
			height,
			memory,
			pitch);
		return true;
#else
		(void)layer;
		(void)mip;
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		(void)data;
		(void)size;
		(void)pitch;
		return false;
#endif
	}

	bool Texture::update3D(
		u8 mip,
		u16 x,
		u16 y,
		u16 z,
		u16 width,
		u16 height,
		u16 depth,
		const void* data,
		u32 size)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| mImmutable
		|| !data
		|| size == 0
		|| width == 0
		|| height == 0
		|| depth == 0)
		{
			return false;
		}

		if (mDescKnown && mDesc.type != TextureType::Texture3D)
			return false;

		if (!region_valid(mip, x, y, z, width, height, depth))
			return false;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return false;

		bgfx::updateTexture3D(
			detail::TextureAccess::native(*this),
			mip,
			x,
			y,
			z,
			width,
			height,
			depth,
			memory);
		return true;
#else
		(void)mip;
		(void)x;
		(void)y;
		(void)z;
		(void)width;
		(void)height;
		(void)depth;
		(void)data;
		(void)size;
		return false;
#endif
	}

	bool Texture::updateCube(
		u16 layer,
		u8 side,
		u8 mip,
		u16 x,
		u16 y,
		u16 width,
		u16 height,
		const void* data,
		u32 size,
		u16 pitch)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| mImmutable
		|| !data
		|| size == 0
		|| width == 0
		|| height == 0
		|| side > 5)
		{
			return false;
		}

		if (mDescKnown && mDesc.type != TextureType::TextureCube)
			return false;

		if (mDescKnown && layer >= mDesc.layers)
			return false;

		if (!region_valid(mip, x, y, 0, width, height, 1))
			return false;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return false;

		bgfx::updateTextureCube(
			detail::TextureAccess::native(*this),
			layer,
			side,
			mip,
			x,
			y,
			width,
			height,
			memory,
			pitch);
		return true;
#else
		(void)layer;
		(void)side;
		(void)mip;
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		(void)data;
		(void)size;
		(void)pitch;
		return false;
#endif
	}

	u32 Texture::read(void* data, u8 mip, u16 layer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid()
		|| !data
		|| (mDesc.flags & TextureFlags::ReadBack) == TextureFlags::None)
		{
			return 0;
		}

		if (!region_valid(mip, 0, 0, layer, 0, 0, 1))
			return 0;

		const bgfx::TextureRegion region{
			detail::TextureAccess::native(*this),
			mip,
			0,
			0,
			layer,
			0,
			0,
			0,
		};

		return bgfx::read(region, data);
#else
		(void)data;
		(void)mip;
		(void)layer;
		return 0;
#endif
	}

	bool Texture::blit(
		u16 view,
		const TextureRegion& dstRegion,
		const TextureRegion& srcRegion)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!dstRegion.texture || !srcRegion.texture)
			return false;

		const Texture& dst{ *dstRegion.texture };
		const Texture& src{ *srcRegion.texture };

		if (!dst.valid() || !src.valid() || &dst == &src)
			return false;

		if (dst.mImmutable
		|| (dst.mDesc.flags & TextureFlags::BlitDestination) == TextureFlags::None)
		{
			return false;
		}

		if (dst.mDescKnown
		&& src.mDescKnown
		&& dst.mDesc.format != src.mDesc.format)
		{
			return false;
		}

		if (!dst.region_valid(
			dstRegion.mip,
			dstRegion.x,
			dstRegion.y,
			dstRegion.z,
			dstRegion.width,
			dstRegion.height,
			dstRegion.depth))
		{
			return false;
		}

		if (!src.region_valid(
			srcRegion.mip,
			srcRegion.x,
			srcRegion.y,
			srcRegion.z,
			srcRegion.width,
			srcRegion.height,
			srcRegion.depth))
		{
			return false;
		}

		const bgfx::Caps* caps{ bgfx::getCaps() };
		if (!caps || view >= caps->limits.maxViews)
			return false;

		bgfx::blit(view, to_bgfx(dstRegion), to_bgfx(srcRegion));
		return true;
#else
		(void)view;
		(void)dstRegion;
		(void)srcRegion;
		return false;
#endif
	}

	bool Texture::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void Texture::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::TextureHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mDesc = {};
		mNumMips = 0;
		mImmutable = false;
		mDescKnown = false;
	}
}
