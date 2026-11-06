#include <wgfx/shader.hpp>

#include "device_internal.hpp"

#include <cstring>
#include <vector>

#if defined(WGFX_BACKEND_BGFX)
#include "shader_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		UniformType from_bgfx(bgfx::UniformType::Enum type)
		{
			switch (type)
			{
			case bgfx::UniformType::Sampler: return UniformType::Sampler;
			case bgfx::UniformType::Vec4: return UniformType::Vec4;
			case bgfx::UniformType::Mat3: return UniformType::Mat3;
			case bgfx::UniformType::Mat4: return UniformType::Mat4;
			case bgfx::UniformType::End:
			case bgfx::UniformType::Count: break;
			}

			return UniformType::Count;
		}
	} // anonymous namespace
#endif

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

	u16 Shader::uniforms(ShaderUniformInfo* uniforms, u16 max) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return 0;

		const bgfx::ShaderHandle handle{ detail::ShaderAccess::native(*this) };
		const u16 count{ bgfx::getShaderUniforms(handle) };
		if (!uniforms || max == 0)
			return count;

		const u16 num{ count < max ? count : max };
		if (num == 0)
			return count;

		std::vector<bgfx::UniformHandle> handles(num);
		bgfx::getShaderUniforms(handle, handles.data(), num);

		for (u16 ii = 0; ii < num; ++ii)
		{
			bgfx::UniformInfo info{};
			bgfx::getUniformInfo(handles[ii], info);

			std::memcpy(uniforms[ii].name, info.name, sizeof(uniforms[ii].name));
			uniforms[ii].type = from_bgfx(info.type);
			uniforms[ii].count = info.num;
		}

		return count;
#else
		(void)uniforms;
		(void)max;
		return 0;
#endif
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
