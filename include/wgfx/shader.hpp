#pragma once

#include <wgfx/uniform.hpp>

#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct ShaderAccess;
	}

	struct ShaderUniformInfo
	{
		static constexpr u32 MaxNameLength{ 255 };

		char name[MaxNameLength + 1]{};
		UniformType type{ UniformType::Count };
		u16 count{ 0 };
	};

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

		u16 uniforms(ShaderUniformInfo* uniforms = nullptr, u16 max = 0) const;

	private:
		friend struct detail::ShaderAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}