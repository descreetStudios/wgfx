#pragma once

#include <wgfx/vertex_layout.hpp>

namespace wgfx
{
	namespace detail
	{
		struct VertexBufferAccess;
	}

	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		~VertexBuffer();

		VertexBuffer(const VertexBuffer&) = delete;
		VertexBuffer& operator=(const VertexBuffer&) = delete;

		VertexBuffer(VertexBuffer&& other) noexcept;
		VertexBuffer& operator=(VertexBuffer&& other) noexcept;

		static VertexBuffer create(
			const VertexLayout& layout,
			const void* data,
			u32 size);

		bool valid() const;
		void reset();

	private:
		friend struct detail::VertexBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
