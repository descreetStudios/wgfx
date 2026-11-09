#include <wgfx/index_buffer.hpp>

#include "device_internal.hpp"

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
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
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
	}
}