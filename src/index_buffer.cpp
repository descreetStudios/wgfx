#include <wgfx/index_buffer.hpp>

#include "device_internal.hpp"

#include <limits>

#if defined(WGFX_BACKEND_BGFX)
#include "index_buffer_internal.hpp"

#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace
	{
		static_assert(static_cast<u16>(BufferFlags::None) == BGFX_BUFFER_NONE,
			"BufferFlags::None must mirror BGFX_BUFFER_NONE");
		static_assert(static_cast<u16>(BufferFlags::ComputeRead) == BGFX_BUFFER_COMPUTE_READ,
			"BufferFlags::ComputeRead must mirror BGFX_BUFFER_COMPUTE_READ");
		static_assert(static_cast<u16>(BufferFlags::ComputeWrite) == BGFX_BUFFER_COMPUTE_WRITE,
			"BufferFlags::ComputeWrite must mirror BGFX_BUFFER_COMPUTE_WRITE");
		static_assert(static_cast<u16>(BufferFlags::ComputeReadWrite) == BGFX_BUFFER_COMPUTE_READ_WRITE,
			"BufferFlags::ComputeReadWrite must mirror BGFX_BUFFER_COMPUTE_READ_WRITE");
		static_assert(static_cast<u16>(BufferFlags::DrawIndirect) == BGFX_BUFFER_DRAW_INDIRECT,
			"BufferFlags::DrawIndirect must mirror BGFX_BUFFER_DRAW_INDIRECT");
		static_assert(static_cast<u16>(BufferFlags::AllowResize) == BGFX_BUFFER_ALLOW_RESIZE,
			"BufferFlags::AllowResize must mirror BGFX_BUFFER_ALLOW_RESIZE");
		static_assert(static_cast<u16>(BufferFlags::Index32) == BGFX_BUFFER_INDEX32,
			"BufferFlags::Index32 must mirror BGFX_BUFFER_INDEX32");

		u16 native_flags(IndexType type, BufferFlags flags)
		{
			u16 result{ static_cast<u16>(flags) };
			if (type == IndexType::Uint32)
				result |= BGFX_BUFFER_INDEX32;
			else
				result &= static_cast<u16>(~BGFX_BUFFER_INDEX32);

			return result;
		}
	} // anonymous namespace
}
#endif

namespace wgfx
{
	IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mCount{ other.mCount }
		, mFlags{ other.mFlags }
		, mIndex32{ other.mIndex32 }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mCount = 0;
		other.mFlags = BufferFlags::None;
		other.mIndex32 = false;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mCount = other.mCount;
		mFlags = other.mFlags;
		mIndex32 = other.mIndex32;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mCount = 0;
		other.mFlags = BufferFlags::None;
		other.mIndex32 = false;
		return *this;
	}

	IndexBuffer::~IndexBuffer()
	{
		reset();
	}

	IndexBuffer IndexBuffer::create(
		const void* data,
		u32 size,
		IndexType type,
		BufferFlags flags)
	{
		IndexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (!data || size == 0)
			return result;

		if (type != IndexType::Uint16 && type != IndexType::Uint32)
			return result;

		const u32 indexSize{ type == IndexType::Uint32
			? static_cast<u32>(sizeof(u32))
			: static_cast<u32>(sizeof(u16)) };
		if (size % indexSize != 0)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return result;

		const bgfx::IndexBufferHandle handle{ bgfx::createIndexBuffer(
			memory, native_flags(type, flags)) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mCount = size / indexSize;
		result.mFlags = flags;
		result.mIndex32 = type == IndexType::Uint32;
#else
		(void)data;
		(void)size;
		(void)type;
		(void)flags;
#endif

		return result;
	}

	bool IndexBuffer::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	bool IndexBuffer::set_name(const char* name, i32 length) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !name)
			return false;

		bgfx::setName(
			detail::IndexBufferAccess::native(*this),
			name,
			length < 0 ? std::numeric_limits<i32>::max() : length);
		return true;
#else
		(void)name;
		(void)length;
		return false;
#endif
	}

	void IndexBuffer::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::IndexBufferHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mCount = 0;
		mFlags = BufferFlags::None;
		mIndex32 = false;
	}

	u32 IndexBuffer::count() const
	{
		return valid() ? mCount : 0;
	}

	bool IndexBuffer::index_32() const
	{
		return valid() && mIndex32;
	}

	bool IndexBuffer::has_flag(BufferFlags flag) const
	{
		if (!valid())
			return false;

		return (static_cast<u16>(mFlags) & static_cast<u16>(flag))
			== static_cast<u16>(flag);
	}
}
