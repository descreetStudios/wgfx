#include <wgfx/index_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "index_buffer_internal.hpp"

#include <bgfx/bgfx.h>
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
		IndexType type)
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

		const u16 flags{ static_cast<u16>(type == IndexType::Uint32
			? BGFX_BUFFER_INDEX32
			: BGFX_BUFFER_NONE) };
		const bgfx::IndexBufferHandle handle{ bgfx::createIndexBuffer(memory, flags) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)data;
		(void)size;
		(void)type;
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
