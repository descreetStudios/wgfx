#pragma once

#include <wgfx/buffer_flags.hpp>
#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct IndexBufferAccess;
	}

	enum class IndexType : u8
	{
		Uint16,
		Uint32,
	};

	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		~IndexBuffer();

		IndexBuffer(const IndexBuffer&) = delete;
		IndexBuffer& operator=(const IndexBuffer&) = delete;

		IndexBuffer(IndexBuffer&& other) noexcept;
		IndexBuffer& operator=(IndexBuffer&& other) noexcept;

		static IndexBuffer create(
			const void* data,
			u32 size,
			IndexType type = IndexType::Uint16,
			BufferFlags flags = BufferFlags::None);

		bool valid() const;
		void reset();

		bool set_name(const char* name, i32 length = -1) const;

		u32 count() const;
		bool index_32() const;
		bool has_flag(BufferFlags flag) const;

	private:
		friend struct detail::IndexBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
		u32 mCount{ 0 };
		BufferFlags mFlags{ BufferFlags::None };
		bool mIndex32{ false };
	};
}
