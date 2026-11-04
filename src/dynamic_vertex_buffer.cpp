#include <wgfx/dynamic_vertex_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "dynamic_vertex_buffer_internal.hpp"
#include "vertex_layout_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
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

		bool has_flag(BufferFlags flags, BufferFlags flag)
		{
			return (static_cast<u16>(flags) & static_cast<u16>(flag)) != 0;
		}
	} // anonymous namespace
#endif

	DynamicVertexBuffer::DynamicVertexBuffer(DynamicVertexBuffer&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mSize{ other.mSize }
		, mStride{ other.mStride }
		, mFlags{ other.mFlags }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mSize = 0;
		other.mStride = 0;
		other.mFlags = 0;
	}

	DynamicVertexBuffer& DynamicVertexBuffer::operator=(
		DynamicVertexBuffer&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mSize = other.mSize;
		mStride = other.mStride;
		mFlags = other.mFlags;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mSize = 0;
		other.mStride = 0;
		other.mFlags = 0;
		return *this;
	}

	DynamicVertexBuffer::~DynamicVertexBuffer()
	{
		reset();
	}

	DynamicVertexBuffer DynamicVertexBuffer::create(
		const VertexLayout& layout,
		u32 vertexCount,
		BufferFlags flags)
	{
		DynamicVertexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (!layout.valid() || layout.stride() == 0 || vertexCount == 0)
			return result;

		const u16 stride{ layout.stride() };
		if (vertexCount > MaxU32 / stride)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::DynamicVertexBufferHandle handle{
			bgfx::createDynamicVertexBuffer(
				vertexCount,
				detail::VertexLayoutAccess::native(layout),
				static_cast<u16>(flags)) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mSize = vertexCount * stride;
		result.mStride = stride;
		result.mFlags = static_cast<u16>(flags);
#else
		(void)layout;
		(void)vertexCount;
		(void)flags;
#endif

		return result;
	}

	DynamicVertexBuffer DynamicVertexBuffer::create(
		const VertexLayout& layout,
		const void* data,
		u32 size,
		BufferFlags flags)
	{
		DynamicVertexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (!data || size == 0 || !layout.valid() || layout.stride() == 0)
			return result;

		const u16 stride{ layout.stride() };
		if (size % stride != 0 || has_flag(flags, BufferFlags::ComputeWrite))
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return result;

		const bgfx::DynamicVertexBufferHandle handle{
			bgfx::createDynamicVertexBuffer(
				memory,
				detail::VertexLayoutAccess::native(layout),
				static_cast<u16>(flags)) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mSize = size;
		result.mStride = stride;
		result.mFlags = static_cast<u16>(flags);
#else
		(void)layout;
		(void)data;
		(void)size;
		(void)flags;
#endif

		return result;
	}

	bool DynamicVertexBuffer::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	bool DynamicVertexBuffer::update(
		const void* data,
		u32 size,
		u32 startVertex)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!data
		|| size == 0
		|| !valid()
		|| mStride == 0
		|| size % mStride != 0
		|| has_flag(static_cast<BufferFlags>(mFlags), BufferFlags::ComputeWrite))
		{
			return false;
		}

		if (startVertex > MaxU32 / mStride)
			return false;

		const u32 offset{ startVertex * mStride };
		if (offset > MaxU32 - size)
			return false;

		const u32 end{ offset + size };
		const bool grows{ end > mSize };
		if (grows)
		{
			// Growing from a non-zero start vertex is not supported.
			if (startVertex != 0 || !has_flag(
				static_cast<BufferFlags>(mFlags), BufferFlags::AllowResize))
			{
				return false;
			}
		}

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return false;

		bgfx::update(
			bgfx::DynamicVertexBufferHandle{ mHandle },
			startVertex,
			memory);

		if (grows)
			mSize = size;

		return true;
#else
		(void)data;
		(void)size;
		(void)startVertex;
		return false;
#endif
	}

	void DynamicVertexBuffer::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::DynamicVertexBufferHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mSize = 0;
		mStride = 0;
		mFlags = 0;
	}
}
