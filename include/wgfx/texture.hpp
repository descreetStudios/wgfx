#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct TextureAccess;
	}

	enum class TextureFormat : u16
	{
		BC1,
		BC2,
		BC3,
		BC4,
		BC4S,
		BC5,
		BC5S,
		BC6H,
		BC6HU,
		BC7,
		ETC1,
		ETC2,
		ETC2A,
		ETC2A1,
		EACR11,
		EACR11S,
		EACRG11,
		EACRG11S,
		PTC12,
		PTC14,
		PTC12A,
		PTC14A,
		PTC22,
		PTC24,
		ATC,
		ATCE,
		ATCI,
		ASTC4x4,
		ASTC5x4,
		ASTC5x5,
		ASTC6x5,
		ASTC6x6,
		ASTC8x5,
		ASTC8x6,
		ASTC8x8,
		ASTC10x5,
		ASTC10x6,
		ASTC10x8,
		ASTC10x10,
		ASTC12x10,
		ASTC12x12,
		Unknown,
		R1,
		A8,
		R8,
		R8I,
		R8U,
		R8S,
		R16,
		R16I,
		R16U,
		R16F,
		R16S,
		R32I,
		R32U,
		R32F,
		RG8,
		RG8I,
		RG8U,
		RG8S,
		RG16,
		RG16I,
		RG16U,
		RG16F,
		RG16S,
		RG32I,
		RG32U,
		RG32F,
		RGB8,
		RGB8I,
		RGB8U,
		RGB8S,
		RGB9E5F,
		BGRA8,
		RGBA8,
		RGBA8I,
		RGBA8U,
		RGBA8S,
		RGBA16,
		RGBA16I,
		RGBA16U,
		RGBA16F,
		RGBA16S,
		RGBA32I,
		RGBA32U,
		RGBA32F,
		B5G6R5,
		R5G6B5,
		BGRA4,
		RGBA4,
		BGR5A1,
		RGB5A1,
		RGB10A2,
		RGB10A2U,
		RG11B10F,
		UnknownDepth,
		D16,
		D24,
		D24S8,
		D32,
		D16F,
		D24F,
		D32F,
		D32FS8,
		D0S8,

		Count,
	};

	enum class TextureFlags : u64
	{
		None = 0,
		MsaaSample = 0x0000000800000000ull,
		RenderTarget = 0x0000001000000000ull,
		RenderTargetMsaaX2 = 0x0000002000000000ull,
		RenderTargetMsaaX4 = 0x0000003000000000ull,
		RenderTargetMsaaX8 = 0x0000004000000000ull,
		RenderTargetMsaaX16 = 0x0000005000000000ull,
		RenderTargetWriteOnly = 0x0000008000000000ull,
		ComputeWrite = 0x0000100000000000ull,
		Srgb = 0x0000200000000000ull,
		BlitDestination = 0x0000400000000000ull,
		ReadBack = 0x0000800000000000ull,
		ExternalShared = 0x0001000000000000ull,
		SrgbMutable = 0x0040000000000000ull,
	};

	constexpr TextureFlags operator|(TextureFlags lhs, TextureFlags rhs) noexcept
	{
		return static_cast<TextureFlags>(
			static_cast<u64>(lhs) | static_cast<u64>(rhs));
	}

	constexpr TextureFlags operator&(TextureFlags lhs, TextureFlags rhs) noexcept
	{
		return static_cast<TextureFlags>(
			static_cast<u64>(lhs) & static_cast<u64>(rhs));
	}

	constexpr TextureFlags& operator|=(TextureFlags& lhs, TextureFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum class TextureType : u8
	{
		Texture2D,
		Texture3D,
	};

	struct TextureDesc
	{
		u16 width{ 0 };
		u16 height{ 0 };
		u16 layers{ 1 };
		TextureFormat format{ TextureFormat::RGBA8 };
		TextureFlags flags{ TextureFlags::None };
		bool mipmapped{ false };

		TextureType type{ TextureType::Texture2D };
		u16 depth{ 1 };
	};

	class Texture
	{
	public:
		Texture() = default;
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		static Texture create(
			const TextureDesc& desc,
			const void* data = nullptr,
			u32 size = 0);

		bool valid() const;
		void reset();

	private:
		friend struct detail::TextureAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
