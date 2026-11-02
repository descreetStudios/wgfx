#pragma once

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
			IndexType type = IndexType::Uint16);

		bool valid() const;
		void reset();

	private:
		friend struct detail::IndexBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
