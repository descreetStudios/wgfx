#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct UniformAccess;
	}

	enum class UniformType : u8
	{
		Sampler,
		Vec4,
		Mat3,
		Mat4,

		Count,
	};

	class Uniform
	{
	public:
		Uniform() = default;
		~Uniform();

		Uniform(const Uniform&) = delete;
		Uniform& operator=(const Uniform&) = delete;

		Uniform(Uniform&& other) noexcept;
		Uniform& operator=(Uniform&& other) noexcept;

		static Uniform create(
			const char* name,
			UniformType type,
			u16 count = 1);

		bool valid() const;
		bool set(const void* data, u16 count = 0xffff) const;
		void reset();

	private:
		friend struct detail::UniformAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
