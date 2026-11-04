#include <wgfx/uniform.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "uniform_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		bool is_identifier(const char* name)
		{
			if (!name || name[0] == '\0')
				return false;

			const auto is_alpha = [](char ch)
			{
				return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
			};

			const auto is_digit = [](char ch)
			{
				return ch >= '0' && ch <= '9';
			};

			if (!is_alpha(name[0]) && name[0] != '_')
				return false;

			for (const char* ch = name + 1; *ch != '\0'; ++ch)
			{
				if (!is_alpha(*ch) && !is_digit(*ch) && *ch != '_')
					return false;
			}

			return true;
		}

		bool is_predefined(const char* name)
		{
			static constexpr const char* Names[]
			{
				"u_viewRect",
				"u_viewTexel",
				"u_view",
				"u_invView",
				"u_proj",
				"u_invProj",
				"u_viewProj",
				"u_invViewProj",
				"u_model",
				"u_modelView",
				"u_invModelView",
				"u_modelViewProj",
				"u_alphaRef4",
				"bgfx_indirectArgBase",
				"bgfx_numWorkGroups",
			};

			for (const char* predefined : Names)
			{
				const char* lhs{ name };
				const char* rhs{ predefined };
				while (*lhs != '\0' && *lhs == *rhs)
				{
					++lhs;
					++rhs;
				}

				if (*lhs == '\0' && *rhs == '\0')
					return true;
			}

			return false;
		}

		bgfx::UniformType::Enum to_bgfx(UniformType type)
		{
			switch (type)
			{
			case UniformType::Sampler: return bgfx::UniformType::Sampler;
			case UniformType::Vec4: return bgfx::UniformType::Vec4;
			case UniformType::Mat3: return bgfx::UniformType::Mat3;
			case UniformType::Mat4: return bgfx::UniformType::Mat4;
			case UniformType::Count: break;
			}

			return bgfx::UniformType::Count;
		}

		bgfx::UniformFreq::Enum to_bgfx(UniformFreq freq)
		{
			switch (freq)
			{
			case UniformFreq::Draw: return bgfx::UniformFreq::Draw;
			case UniformFreq::View: return bgfx::UniformFreq::View;
			case UniformFreq::Frame: return bgfx::UniformFreq::Frame;
			case UniformFreq::Count: break;
			}

			return bgfx::UniformFreq::Count;
		}
	} // anonymous namespace
#endif

	Uniform::Uniform(Uniform&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
		, mFreq{ other.mFreq }
		, mCount{ other.mCount }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mFreq = UniformFreq::Draw;
		other.mCount = 1;
	}

	Uniform& Uniform::operator=(Uniform&& other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		mHandle = other.mHandle;
		mGeneration = other.mGeneration;
		mFreq = other.mFreq;
		mCount = other.mCount;
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
		other.mFreq = UniformFreq::Draw;
		other.mCount = 1;
		return *this;
	}

	Uniform::~Uniform()
	{
		reset();
	}

	Uniform Uniform::create(
		const char* name,
		UniformType type,
		u16 count)
	{
		return create(name, UniformFreq::Draw, type, count);
	}

	Uniform Uniform::create(
		const char* name,
		UniformFreq freq,
		UniformType type,
		u16 count)
	{
		Uniform result;

#if defined(WGFX_BACKEND_BGFX)
		if (!is_identifier(name) || is_predefined(name) || count == 0)
			return result;

		const bgfx::UniformType::Enum nativeType{ to_bgfx(type) };
		if (nativeType == bgfx::UniformType::Count)
			return result;

		const bgfx::UniformFreq::Enum nativeFreq{ to_bgfx(freq) };
		if (nativeFreq == bgfx::UniformFreq::Count)
			return result;

		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::UniformHandle handle{ bgfx::createUniform(
			name, nativeFreq, nativeType, count) };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
		result.mFreq = freq;
		result.mCount = count;
#else
		(void)name;
		(void)freq;
		(void)type;
		(void)count;
#endif

		return result;
	}

	bool Uniform::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	UniformFreq Uniform::frequency() const
	{
		return mFreq;
	}

	bool Uniform::set(const void* data, u16 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!data || !valid() || mFreq != UniformFreq::Draw)
			return false;

		const u16 num{ count == MaxU16 ? mCount : count };
		if (num == 0 || num > mCount)
			return false;

		bgfx::setUniform(
			bgfx::UniformHandle{ mHandle },
			data,
			num);
		return true;
#else
		(void)data;
		(void)count;
		return false;
#endif
	}

	bool Uniform::setView(
		u16 viewId,
		const void* data,
		u16 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!data || !valid() || mFreq != UniformFreq::View)
			return false;

		const u16 num{ count == MaxU16 ? mCount : count };
		if (num == 0 || num > mCount)
			return false;

		bgfx::setViewUniform(
			static_cast<bgfx::ViewId>(viewId),
			bgfx::UniformHandle{ mHandle },
			data,
			num);
		return true;
#else
		(void)viewId;
		(void)data;
		(void)count;
		return false;
#endif
	}

	bool Uniform::setFrame(const void* data, u16 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!data || !valid() || mFreq != UniformFreq::Frame)
			return false;

		const u16 num{ count == MaxU16 ? mCount : count };
		if (num == 0 || num > mCount)
			return false;

		bgfx::setFrameUniform(
			bgfx::UniformHandle{ mHandle },
			data,
			num);
		return true;
#else
		(void)data;
		(void)count;
		return false;
#endif
	}

	void Uniform::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::UniformHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
		mFreq = UniformFreq::Draw;
		mCount = 1;
	}
}
