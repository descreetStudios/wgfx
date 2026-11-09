#include <wgfx/dynamic_index_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "dynamic_index_buffer_internal.hpp"

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
		static_assert(static_cast<u16>(BufferFlags::DrawIndirect) == BGFX_BUFFER_DRAW_INDIRECT,
			"BufferFlags::DrawIndirect must mirror BGFX_BUFFER_DRAW_INDIRECT");
		static_assert(static_cast<u16>(BufferFlags::AllowResize) == BGFX_BUFFER_ALLOW_RESIZE,
			"BufferFlags::AllowResize must mirror BGFX_BUFFER_ALLOW_RESIZE");
		static_assert(static_cast<u16>(BufferFlags::Index32) == BGFX_BUFFER_INDEX32,
			"BufferFlags::Index32 must mirror BGFX_BUFFER_INDEX32");

		bool has_flag(BufferFlags flags, BufferFlags flag)
		{
			return (static_cast<u16>(flags) & static_cast<u16>(flag)) != 0;
		}

		u32 index_size(IndexType type)
		{
			switch (type)
			{
			case IndexType::Uint16: return static_cast<u32>(sizeof(u16));
			case IndexType::Uint32: return static_cast<u32>(sizeof(u32));
			}

			return 0;
		}

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
#endif

	DynamicIndexBuffer::DynamicIndexBuffer(DynamicIndexBuffer&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mSize{ other.mSize }
		, mIndexSize{ other.mIndexSize }
		, mFlags{ other.mFlags }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mSize = 0;
		other.mIndexSize = 0;
		other.mFlags = 0;
	}

	DynamicIndexBuffer& DynamicIndexBuffer::operator=(
		DynamicIndexBuffer&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mSize = other.mSize;
		mIndexSize = other.mIndexSize;
		mFlags = other.mFlags;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mSize = 0;
		other.mIndexSize = 0;
		other.mFlags = 0;
		return *this;
	}

	DynamicIndexBuffer::~DynamicIndexBuffer()
	{
		reset();
	}

	DynamicIndexBuffer DynamicIndexBuffer::create(
		u32 indexCount,
		IndexType type,
		BufferFlags flags)
	{
		DynamicIndexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		const u32 indexSize{ index_size(type) };
		if (indexCount == 0
		|| indexSize == 0
		|| indexCount > MaxU32 / indexSize)
		{
			return result;
		}

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const u16 bufferFlags{ native_flags(type, flags) };
		const bgfx::DynamicIndexBufferHandle handle{
			bgfx::createDynamicIndexBuffer(indexCount, bufferFlags) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mSize = indexCount * indexSize;
		result.mIndexSize = static_cast<u16>(indexSize);
		result.mFlags = bufferFlags;
#else
		(void)indexCount;
		(void)type;
		(void)flags;
#endif

		return result;
	}

	DynamicIndexBuffer DynamicIndexBuffer::create(
		const void* data,
		u32 size,
		IndexType type,
		BufferFlags flags)
	{
		DynamicIndexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		const u32 indexSize{ index_size(type) };
		if (!data
		|| size == 0
		|| indexSize == 0
		|| size % indexSize != 0
		|| has_flag(flags, BufferFlags::ComputeWrite))
		{
			return result;
		}

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return result;

		const u16 bufferFlags{ native_flags(type, flags) };
		const bgfx::DynamicIndexBufferHandle handle{
			bgfx::createDynamicIndexBuffer(memory, bufferFlags) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mSize = size;
		result.mIndexSize = static_cast<u16>(indexSize);
		result.mFlags = bufferFlags;
#else
		(void)data;
		(void)size;
		(void)type;
		(void)flags;
#endif

		return result;
	}

	bool DynamicIndexBuffer::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	bool DynamicIndexBuffer::update(
		const void* data,
		u32 size,
		u32 startIndex)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!data
		|| size == 0
		|| !valid()
		|| mIndexSize == 0
		|| size % mIndexSize != 0
		|| has_flag(static_cast<BufferFlags>(mFlags),
			BufferFlags::ComputeWrite))
		{
			return false;
		}

		if (startIndex > MaxU32 / mIndexSize)
			return false;

		const u32 offset{ startIndex * mIndexSize };
		if (offset > MaxU32 - size)
			return false;

		const u32 end{ offset + size };
		const bool grows{ end > mSize };
		if (grows)
		{
			// Growing from a non-zero start index is not supported.
			if (startIndex != 0 || !has_flag(
				static_cast<BufferFlags>(mFlags),
				BufferFlags::AllowResize))
			{
				return false;
			}
		}

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return false;

		bgfx::update(
			bgfx::DynamicIndexBufferHandle{ mHandle },
			startIndex,
			memory);

		if (grows)
			mSize = size;

		return true;
#else
		(void)data;
		(void)size;
		(void)startIndex;
		return false;
#endif
	}

	void DynamicIndexBuffer::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::DynamicIndexBufferHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mSize = 0;
		mIndexSize = 0;
		mFlags = 0;
	}
}
