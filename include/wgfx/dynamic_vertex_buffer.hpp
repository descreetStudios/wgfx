#pragma once

#include <wgfx/buffer_flags.hpp>
#include <wgfx/vertex_layout.hpp>

namespace wgfx
{
	namespace detail
	{
		struct DynamicVertexBufferAccess;
	}

	class DynamicVertexBuffer
	{
	public:
		DynamicVertexBuffer() = default;
		~DynamicVertexBuffer();

		DynamicVertexBuffer(const DynamicVertexBuffer&) = delete;
		DynamicVertexBuffer& operator=(const DynamicVertexBuffer&) = delete;

		DynamicVertexBuffer(DynamicVertexBuffer&& other) noexcept;
		DynamicVertexBuffer& operator=(DynamicVertexBuffer&& other) noexcept;

		static DynamicVertexBuffer create(
			const VertexLayout& layout,
			u32 vertexCount,
			BufferFlags flags = BufferFlags::None);
		static DynamicVertexBuffer create(
			const VertexLayout& layout,
			const void* data,
			u32 size,
			BufferFlags flags = BufferFlags::None);

		bool valid() const;
		bool update(const void* data, u32 size, u32 startVertex = 0);
		void reset();

	private:
		friend struct detail::DynamicVertexBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };
		static constexpr u32 MaxU32{ 0xffffffffu };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
		u32 mSize{ 0 };
		u16 mStride{ 0 };
		u16 mFlags{ 0 };
	};
}
