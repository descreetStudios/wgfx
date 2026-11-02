#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct ShaderAccess;
	}

	class Shader
	{
	public:
		Shader() = default;
		~Shader();

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		Shader(Shader&& other) noexcept;
		Shader& operator=(Shader&& other) noexcept;

		static Shader create(const void* data, u32 size);

		bool valid() const;
		void reset();

	private:
		friend struct detail::ShaderAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
