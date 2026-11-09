#pragma once

#include <wgfx/transient_buffer.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		struct TransientBufferAccess
		{
			static bgfx::TransientVertexBuffer native(
				const TransientVertexBuffer& buffer)
			{
				bgfx::TransientVertexBuffer result;
				result.data = static_cast<u8*>(buffer.mData);
				result.size = buffer.mSize;
				result.startVertex = buffer.mStartVertex;
				result.stride = buffer.mStride;
				result.handle = bgfx::VertexBufferHandle{ buffer.mHandle };
				result.layoutHandle = bgfx::VertexLayoutHandle{ buffer.mLayoutHandle };
				return result;
			}

			static bgfx::TransientIndexBuffer native(
				const TransientIndexBuffer& buffer)
			{
				bgfx::TransientIndexBuffer result;
				result.data = static_cast<u8*>(buffer.mData);
				result.size = buffer.mSize;
				result.startIndex = buffer.mStartIndex;
				result.handle = bgfx::IndexBufferHandle{ buffer.mHandle };
				result.isIndex16 = !buffer.mIndex32;
				return result;
			}

			static bgfx::InstanceDataBuffer native(
				const InstanceDataBuffer& buffer)
			{
				bgfx::InstanceDataBuffer result;
				result.data = static_cast<u8*>(buffer.mData);
				result.size = buffer.mSize;
				result.offset = buffer.mOffset;
				result.num = buffer.mNum;
				result.stride = buffer.mStride;
				result.handle = bgfx::VertexBufferHandle{ buffer.mHandle };
				return result;
			}
		};
	} // namespace detail
}
#endif
