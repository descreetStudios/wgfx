#include <wgfx/framebuffer.hpp>

#include "device_internal.hpp"

#include <utility>

#if defined(WGFX_BACKEND_BGFX)
#include "framebuffer_internal.hpp"
#include "texture_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		static_assert(static_cast<u8>(AttachmentAccess::Read) == bgfx::Access::Read,
			"AttachmentAccess::Read must mirror bgfx::Access::Read");
		static_assert(static_cast<u8>(AttachmentAccess::Write) == bgfx::Access::Write,
			"AttachmentAccess::Write must mirror bgfx::Access::Write");
		static_assert(static_cast<u8>(AttachmentAccess::ReadWrite) == bgfx::Access::ReadWrite,
			"AttachmentAccess::ReadWrite must mirror bgfx::Access::ReadWrite");

		static_assert(static_cast<u8>(AttachmentFlags::None) == BGFX_ATTACHMENT_NONE,
			"AttachmentFlags::None must mirror BGFX_ATTACHMENT_NONE");
		static_assert(static_cast<u8>(AttachmentFlags::AutoGenMips) == BGFX_ATTACHMENT_AUTO_GEN_MIPS,
			"AttachmentFlags::AutoGenMips must mirror BGFX_ATTACHMENT_AUTO_GEN_MIPS");
		static_assert(static_cast<u8>(AttachmentFlags::ReadOnlyDepth) == BGFX_ATTACHMENT_READ_ONLY_DEPTH,
			"AttachmentFlags::ReadOnlyDepth must mirror BGFX_ATTACHMENT_READ_ONLY_DEPTH");
		static_assert(static_cast<u8>(AttachmentFlags::ReadOnlyStencil) == BGFX_ATTACHMENT_READ_ONLY_STENCIL,
			"AttachmentFlags::ReadOnlyStencil must mirror BGFX_ATTACHMENT_READ_ONLY_STENCIL");
		static_assert(static_cast<u8>(AttachmentFlags::Srgb) == BGFX_ATTACHMENT_SRGB,
			"AttachmentFlags::Srgb must mirror BGFX_ATTACHMENT_SRGB");

		constexpr u32 MaxAttachments{ 8 };
		constexpr u64 RenderTargetMsaaMask{ 0x0000007000000000ull };

		bool is_depth_format(TextureFormat format)
		{
			switch (format)
			{
			case TextureFormat::D16:
			case TextureFormat::D24:
			case TextureFormat::D24S8:
			case TextureFormat::D32:
			case TextureFormat::D16F:
			case TextureFormat::D24F:
			case TextureFormat::D32F:
			case TextureFormat::D32FS8:
			case TextureFormat::D0S8:
				return true;
			default:
				return false;
			}
		}

		TextureFlags render_target_flags(TextureFlags flags)
		{
			if ((static_cast<u64>(flags) & RenderTargetMsaaMask) == 0)
				flags |= TextureFlags::RenderTarget;

			return flags;
		}

		bgfx::Attachment to_bgfx(const FrameBufferAttachment& attachment)
		{
			bgfx::Attachment result;
			result.init(
				detail::TextureAccess::native(*attachment.texture),
				static_cast<bgfx::Access::Enum>(attachment.access),
				attachment.layer,
				attachment.layers,
				attachment.mip,
				static_cast<u8>(attachment.flags));
			return result;
		}
	} // anonymous namespace
#endif

	FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mColorTexture{ std::move(other.mColorTexture) }
		, mDepthTexture{ std::move(other.mDepthTexture) }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mColorTexture = std::move(other.mColorTexture);
		mDepthTexture = std::move(other.mDepthTexture);
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		return *this;
	}

	FrameBuffer::~FrameBuffer()
	{
		reset();
	}

	FrameBuffer FrameBuffer::create(const FrameBufferDesc& desc)
	{
		FrameBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (desc.width == 0
		|| desc.height == 0
		|| desc.colorFormat >= TextureFormat::Count
		|| is_depth_format(desc.colorFormat))
		{
			return result;
		}

		if (desc.depthFormat != TextureFormat::Count
		&& (desc.depthFormat >= TextureFormat::Count
		|| !is_depth_format(desc.depthFormat)))
		{
			return result;
		}

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		TextureDesc colorDesc;
		colorDesc.width = desc.width;
		colorDesc.height = desc.height;
		colorDesc.format = desc.colorFormat;
		colorDesc.flags = render_target_flags(desc.flags);

		Texture color{ Texture::create(colorDesc) };
		if (!color.valid())
			return result;

		Texture depth;
		if (desc.depthFormat != TextureFormat::Count)
		{
			TextureDesc depthDesc{ colorDesc };
			depthDesc.format = desc.depthFormat;
			depth = Texture::create(depthDesc);
			if (!depth.valid())
				return result;
		}

		bgfx::TextureHandle handles[2]{
			detail::TextureAccess::native(color),
			detail::TextureAccess::native(depth),
		};
		const u8 count{ static_cast<u8>(
			desc.depthFormat != TextureFormat::Count ? 2 : 1) };

		const bgfx::FrameBufferHandle handle{
			bgfx::createFrameBuffer(count, handles, false) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mColorTexture = std::move(color);
		result.mDepthTexture = std::move(depth);
#else
		(void)desc;
#endif

		return result;
	}

	FrameBuffer FrameBuffer::create(
		const FrameBufferAttachment* attachments,
		u32 count)
	{
		FrameBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (!attachments || count == 0 || count > MaxAttachments)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		bgfx::Attachment nativeAttachments[MaxAttachments];
		for (u32 index = 0; index < count; ++index)
		{
			const FrameBufferAttachment& attachment = attachments[index];
			if (!attachment.texture
			|| !attachment.texture->valid()
			|| attachment.layers == 0)
			{
				return result;
			}

			nativeAttachments[index] = to_bgfx(attachment);
		}

		const u8 attachmentCount{ static_cast<u8>(count) };
		if (!bgfx::isFrameBufferValid(attachmentCount, nativeAttachments))
			return result;

		const bgfx::FrameBufferHandle handle{
			bgfx::createFrameBuffer(
				attachmentCount,
				nativeAttachments,
				false) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)attachments;
		(void)count;
#endif

		return result;
	}

	bool FrameBuffer::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	const Texture& FrameBuffer::colorTexture() const
	{
		return mColorTexture;
	}

	const Texture& FrameBuffer::depthTexture() const
	{
		return mDepthTexture;
	}

	void FrameBuffer::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::FrameBufferHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mColorTexture.reset();
		mDepthTexture.reset();
	}
}
