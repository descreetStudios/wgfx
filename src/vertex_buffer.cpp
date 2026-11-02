#include <wgfx/vertex_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "vertex_buffer_internal.hpp"
#include "vertex_layout_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
	VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
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

	VertexBuffer::~VertexBuffer()
	{
		reset();
	}

	VertexBuffer VertexBuffer::create(
		const VertexLayout& layout,
		const void* data,
		u32 size)
	{
		VertexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (!data || size == 0 || !layout.valid() || layout.stride() == 0)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return result;

		const bgfx::VertexBufferHandle handle{ bgfx::createVertexBuffer(
			memory, detail::VertexLayoutAccess::native(layout)) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)layout;
		(void)data;
		(void)size;
#endif

		return result;
	}

	bool VertexBuffer::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void VertexBuffer::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::VertexBufferHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
	}
}
