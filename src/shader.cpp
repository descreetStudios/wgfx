#include <wgfx/shader.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "shader_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
	Shader::Shader(Shader&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	Shader& Shader::operator=(Shader&& other) noexcept
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

	Shader::~Shader()
	{
		reset();
	}

	Shader Shader::create(const void* data, u32 size)
	{
		Shader result;

#if defined(WGFX_BACKEND_BGFX)
		if (!data || size == 0)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::Memory* memory{ bgfx::copy(data, size) };
		if (!memory)
			return result;

		const bgfx::ShaderHandle handle{ bgfx::createShader(memory) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#else
		(void)data;
		(void)size;
#endif

		return result;
	}

	bool Shader::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void Shader::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::ShaderHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
	}
}
