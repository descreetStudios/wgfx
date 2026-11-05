#include <wgfx/texture.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "texture_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
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
	} // anonymous namespace
#endif

	Texture::Texture(Texture&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
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
#else
		(void)desc;
		(void)data;
		(void)size;
#endif

		return result;
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
	}
}
