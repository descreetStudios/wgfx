#include <wgfx/transient_buffer.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "transient_buffer_internal.hpp"
#include "vertex_layout_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		bool supports_index32()
		{
			const bgfx::Caps* caps{ bgfx::getCaps() };
			return caps && 0 != (caps->supported & BGFX_CAPS_INDEX32);
		}
	} // anonymous namespace
#endif

	u32 TransientVertexBuffer::available(
		u32 numVertices,
		const VertexLayout& layout)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!detail::device_active()
			|| numVertices == 0
			|| !layout.valid()
			|| layout.stride() == 0)
		{
			return 0;
		}

		return bgfx::getAvailTransientVertexBuffer(
			numVertices,
			detail::VertexLayoutAccess::native(layout));
#else
		(void)numVertices;
		(void)layout;
		return 0;
#endif
	}

	TransientVertexBuffer TransientVertexBuffer::alloc(
		u32 numVertices,
		const VertexLayout& layout)
	{
		TransientVertexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (numVertices == 0 || available(numVertices, layout) < numVertices)
			return result;

		bgfx::TransientVertexBuffer native;
		bgfx::allocTransientVertexBuffer(
			&native,
			numVertices,
			detail::VertexLayoutAccess::native(layout));

		result.mData = native.data;
		result.mSize = native.size;
		result.mStartVertex = native.startVertex;
		result.mStride = native.stride;
		result.mHandle = native.handle.idx;
		result.mLayoutHandle = native.layoutHandle.idx;
		result.mGeneration = detail::device_generation();
#else
		(void)numVertices;
		(void)layout;
#endif

		return result;
	}

	bool TransientVertexBuffer::valid() const
	{
		return mData != nullptr
			&& mHandle != InvalidHandle
			&& mSize != 0
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void* TransientVertexBuffer::data()
	{
		return valid() ? mData : nullptr;
	}

	const void* TransientVertexBuffer::data() const
	{
		return valid() ? mData : nullptr;
	}

	u32 TransientVertexBuffer::size() const
	{
		return valid() ? mSize : 0;
	}

	u32 TransientVertexBuffer::count() const
	{
		return valid() && mStride != 0 ? mSize / mStride : 0;
	}

	u32 TransientVertexBuffer::start_vertex() const
	{
		return valid() ? mStartVertex : 0;
	}

	u16 TransientVertexBuffer::stride() const
	{
		return valid() ? mStride : 0;
	}

	u32 TransientIndexBuffer::available(u32 numIndices, bool index32)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!detail::device_active()
			|| numIndices == 0
			|| (index32 && !supports_index32()))
		{
			return 0;
		}

		return bgfx::getAvailTransientIndexBuffer(numIndices, index32);
#else
		(void)numIndices;
		(void)index32;
		return 0;
#endif
	}

	TransientIndexBuffer TransientIndexBuffer::alloc(
		u32 numIndices,
		bool index32)
	{
		TransientIndexBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (numIndices == 0 || available(numIndices, index32) < numIndices)
			return result;

		bgfx::TransientIndexBuffer native;
		bgfx::allocTransientIndexBuffer(&native, numIndices, index32);

		result.mData = native.data;
		result.mSize = native.size;
		result.mStartIndex = native.startIndex;
		result.mHandle = native.handle.idx;
		result.mIndex32 = !native.isIndex16;
		result.mGeneration = detail::device_generation();
#else
		(void)numIndices;
		(void)index32;
#endif

		return result;
	}

	bool TransientIndexBuffer::valid() const
	{
		return mData != nullptr
			&& mHandle != InvalidHandle
			&& mSize != 0
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void* TransientIndexBuffer::data()
	{
		return valid() ? mData : nullptr;
	}

	const void* TransientIndexBuffer::data() const
	{
		return valid() ? mData : nullptr;
	}

	u32 TransientIndexBuffer::size() const
	{
		return valid() ? mSize : 0;
	}

	u32 TransientIndexBuffer::count() const
	{
		if (!valid())
			return 0;

		return mSize / (mIndex32 ? 4u : 2u);
	}

	u32 TransientIndexBuffer::start_index() const
	{
		return valid() ? mStartIndex : 0;
	}

	bool TransientIndexBuffer::index_32() const
	{
		return valid() && mIndex32;
	}

	u32 InstanceDataBuffer::available(u32 numInstances, u16 stride)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!detail::device_active()
			|| numInstances == 0
			|| stride == 0
			|| (stride % 16) != 0)
		{
			return 0;
		}

		return bgfx::getAvailInstanceDataBuffer(numInstances, stride);
#else
		(void)numInstances;
		(void)stride;
		return 0;
#endif
	}

	InstanceDataBuffer InstanceDataBuffer::alloc(u32 numInstances, u16 stride)
	{
		InstanceDataBuffer result;

#if defined(WGFX_BACKEND_BGFX)
		if (numInstances == 0 || stride == 0
			|| available(numInstances, stride) < numInstances)
		{
			return result;
		}

		bgfx::InstanceDataBuffer native;
		bgfx::allocInstanceDataBuffer(&native, numInstances, stride);

		result.mData = native.data;
		result.mSize = native.size;
		result.mOffset = native.offset;
		result.mNum = native.num;
		result.mStride = native.stride;
		result.mHandle = native.handle.idx;
		result.mGeneration = detail::device_generation();
#else
		(void)numInstances;
		(void)stride;
#endif

		return result;
	}

	bool InstanceDataBuffer::valid() const
	{
		return mData != nullptr
			&& mHandle != InvalidHandle
			&& mNum != 0
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void* InstanceDataBuffer::data()
	{
		return valid() ? mData : nullptr;
	}

	const void* InstanceDataBuffer::data() const
	{
		return valid() ? mData : nullptr;
	}

	u32 InstanceDataBuffer::size() const
	{
		return valid() ? mSize : 0;
	}

	u32 InstanceDataBuffer::count() const
	{
		return valid() ? mNum : 0;
	}

	u32 InstanceDataBuffer::offset() const
	{
		return valid() ? mOffset : 0;
	}

	u16 InstanceDataBuffer::stride() const
	{
		return valid() ? mStride : 0;
	}
}