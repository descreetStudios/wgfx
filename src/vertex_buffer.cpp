#include <wgfx/vertex_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "vertex_buffer_internal.hpp"
#include "vertex_layout_internal.hpp"

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
	} // anonymous namespace
}
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
		u32 size,
		BufferFlags flags)
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
			memory, detail::VertexLayoutAccess::native(layout),
			static_cast<u16>(flags)) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)layout;
		(void)data;
		(void)size;
		(void)flags;
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