#pragma once

#include <wgfx/uniform.hpp>

#include <wcore/types.hpp>

#include <filesystem>

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
		static Shader load(const std::filesystem::path& path);

		bool valid() const;
		void reset();

		bool set_name(const char* name, i32 length = -1) const;

		u16 uniforms(ShaderUniformInfo* uniforms = nullptr, u16 max = 0) const;

	private:
		friend struct detail::ShaderAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}