#pragma once

#include <wgfx/texture.hpp>

namespace wgfx
{
	namespace detail
	{
		struct FrameBufferAccess;
	}

	enum class AttachmentAccess : u8
	{
		Read,
		Write,
		ReadWrite,
	};

	enum class AttachmentFlags : u8
	{
		None = 0x00,
		AutoGenMips = 0x01,
		ReadOnlyDepth = 0x02,
		ReadOnlyStencil = 0x04,
		Srgb = 0x08,
	};

	constexpr AttachmentFlags operator|(AttachmentFlags lhs, AttachmentFlags rhs) noexcept
	{
		return static_cast<AttachmentFlags>(
			static_cast<u8>(lhs) | static_cast<u8>(rhs));
	}

	constexpr AttachmentFlags operator&(AttachmentFlags lhs, AttachmentFlags rhs) noexcept
	{
		return static_cast<AttachmentFlags>(
			static_cast<u8>(lhs) & static_cast<u8>(rhs));
	}

	constexpr AttachmentFlags& operator|=(AttachmentFlags& lhs, AttachmentFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	struct FrameBufferAttachment
	{
		const Texture* texture{ nullptr };
		AttachmentAccess access{ AttachmentAccess::Write };
		AttachmentFlags flags{ AttachmentFlags::None };
		u16 mip{ 0 };
		u16 layer{ 0 };
		u16 layers{ 1 };
	};

	struct FrameBufferDesc
	{
		u16 width{ 0 };
		u16 height{ 0 };
		TextureFormat colorFormat{ TextureFormat::RGBA8 };

		// TextureFormat::Count disables the depth-stencil attachment.
		TextureFormat depthFormat{ TextureFormat::Count };
		TextureFlags flags{ TextureFlags::None };
	};

	class FrameBuffer
	{
	public:
		FrameBuffer() = default;
		~FrameBuffer();

		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer& operator=(const FrameBuffer&) = delete;

		FrameBuffer(FrameBuffer&& other) noexcept;
		FrameBuffer& operator=(FrameBuffer&& other) noexcept;

		static FrameBuffer create(const FrameBufferDesc& desc);
		static FrameBuffer create(
			const FrameBufferAttachment* attachments,
			u32 count);

		bool valid() const;
		void reset();

	private:
		friend struct detail::FrameBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
