#pragma once

#include <wgfx/shader.hpp>

namespace wgfx
{
	namespace detail
	{
		struct ProgramAccess;
	}

	class Program
	{
	public:
		Program() = default;
		~Program();

		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;

		Program(Program&& other) noexcept;
		Program& operator=(Program&& other) noexcept;

		static Program create(const Shader& vertex, const Shader& fragment);
		static Program create_compute(const Shader& compute);

		bool valid() const;
		void reset();

	private:
		friend struct detail::ProgramAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
