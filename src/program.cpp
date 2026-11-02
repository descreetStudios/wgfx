#include <wgfx/program.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "program_internal.hpp"
#include "shader_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
	Program::Program(Program&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	Program& Program::operator=(Program&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		return *this;
	}

	Program::~Program()
	{
		reset();
	}

	Program Program::create(const Shader& vertex, const Shader& fragment)
	{
		Program result;

#if defined(WGFX_BACKEND_BGFX)
		if (!vertex.valid() || !fragment.valid())
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::ProgramHandle handle{ bgfx::createProgram(
			detail::ShaderAccess::native(vertex),
			detail::ShaderAccess::native(fragment),
			false) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)vertex;
		(void)fragment;
#endif

		return result;
	}

	Program Program::create_compute(const Shader& compute)
	{
		Program result;

#if defined(WGFX_BACKEND_BGFX)
		if (!compute.valid())
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::ProgramHandle handle{ bgfx::createProgram(
			detail::ShaderAccess::native(compute),
			false) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)compute;
#endif

		return result;
	}

	bool Program::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void Program::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::ProgramHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
	}
}
