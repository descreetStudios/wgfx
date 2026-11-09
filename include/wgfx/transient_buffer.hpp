#pragma once

#include <wgfx/vertex_layout.hpp>

namespace wgfx
{
	namespace detail
	{
		struct TransientBufferAccess;
	}

	class TransientVertexBuffer
	{
	public:
		TransientVertexBuffer() = default;

		static u32 available(u32 numVertices, const VertexLayout& layout);
		static TransientVertexBuffer alloc(u32 numVertices, const VertexLayout& layout);

		bool valid() const;

		void* data();
		const void* data() const;

		u32 size() const;
		u32 count() const;
		u32 start_vertex() const;
		u16 stride() const;

	private:
		friend struct detail::TransientBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		void* mData{ nullptr };
		u32 mSize{ 0 };
		u32 mStartVertex{ 0 };
		u16 mStride{ 0 };
		u16 mHandle{ InvalidHandle };
		u16 mLayoutHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};

	class TransientIndexBuffer
	{
	public:
		TransientIndexBuffer() = default;

		static u32 available(u32 numIndices, bool index32 = false);
		static TransientIndexBuffer alloc(u32 numIndices, bool index32 = false);

		bool valid() const;

		void* data();
		const void* data() const;

		u32 size() const;
		u32 count() const;
		u32 start_index() const;
		bool index_32() const;

	private:
		friend struct detail::TransientBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		void* mData{ nullptr };
		u32 mSize{ 0 };
		u32 mStartIndex{ 0 };
		u16 mHandle{ InvalidHandle };
		bool mIndex32{ false };
		u32 mGeneration{ 0 };
	};

	class InstanceDataBuffer
	{
	public:
		InstanceDataBuffer() = default;

		static u32 available(u32 numInstances, u16 stride);
		static InstanceDataBuffer alloc(u32 numInstances, u16 stride);

		bool valid() const;

		void* data();
		const void* data() const;

		u32 size() const;
		u32 count() const;
		u32 offset() const;
		u16 stride() const;

	private:
		friend struct detail::TransientBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		void* mData{ nullptr };
		u32 mSize{ 0 };
		u32 mOffset{ 0 };
		u32 mNum{ 0 };
		u16 mStride{ 0 };
		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}