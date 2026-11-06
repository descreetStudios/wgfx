#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct IndirectBufferAccess;
	}

	class IndirectBuffer
	{
	public:
		IndirectBuffer() = default;
		~IndirectBuffer();

		IndirectBuffer(const IndirectBuffer&) = delete;
		IndirectBuffer& operator=(const IndirectBuffer&) = delete;

		IndirectBuffer(IndirectBuffer&& other) noexcept;
		IndirectBuffer& operator=(IndirectBuffer&& other) noexcept;

		static IndirectBuffer create(u32 count);

		bool valid() const;
		void reset();

		u32 count() const;

	private:
		friend struct detail::IndirectBufferAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
		u32 mCount{ 0 };
	};
}