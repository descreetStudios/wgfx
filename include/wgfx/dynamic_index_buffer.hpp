#pragma once

#include <wgfx/buffer_flags.hpp>
#include <wgfx/index_buffer.hpp>

namespace wgfx
{
	namespace detail
	{
		struct DynamicIndexBufferAccess;
	}

	class DynamicIndexBuffer
	{
	public:
		DynamicIndexBuffer() = default;
		~DynamicIndexBuffer();

		DynamicIndexBuffer(const DynamicIndexBuffer&) = delete;
		DynamicIndexBuffer& operator=(const DynamicIndexBuffer&) = delete;

		DynamicIndexBuffer(DynamicIndexBuffer&& other) noexcept;
		DynamicIndexBuffer& operator=(DynamicIndexBuffer&& other) noexcept;

		static DynamicIndexBuffer create(
			u32 indexCount,
			IndexType type = IndexType::Uint16,
			BufferFlags flags = BufferFlags::None);
		static DynamicIndexBuffer create(
			const void* data,
			u32 size,
			IndexType type = IndexType::Uint16,
			BufferFlags flags = BufferFlags::None);

		bool valid() const;
		bool update(const void* data, u32 size, u32 startIndex = 0);
		void reset();

	private:
		friend struct detail::DynamicIndexBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };
		static constexpr u32 MaxU32{ 0xffffffffu };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
		u32 mSize{ 0 };
		u16 mIndexSize{ 0 };
		u16 mFlags{ 0 };
	};
}
