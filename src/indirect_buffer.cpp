#include <wgfx/indirect_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "indirect_buffer_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
	IndirectBuffer::IndirectBuffer(IndirectBuffer&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mCount{ other.mCount }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mCount = 0;
	}

	IndirectBuffer& IndirectBuffer::operator=(IndirectBuffer&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mCount = other.mCount;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mCount = 0;
		return *this;
	}

	IndirectBuffer::~IndirectBuffer()
	{
		reset();
	}

	IndirectBuffer IndirectBuffer::create(u32 count)
	{
		IndirectBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (count == 0)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::IndirectBufferHandle handle{
			bgfx::createIndirectBuffer(count) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mCount = count;
#else
		(void)count;
#endif

		return result;
	}

	bool IndirectBuffer::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void IndirectBuffer::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::IndirectBufferHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mCount = 0;
	}

	u32 IndirectBuffer::count() const
	{
		return valid() ? mCount : 0;
	}
}