#include <wgfx/occlusion_query.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "occlusion_query_internal.hpp"

#include <bgfx/bgfx.h>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		static_assert(static_cast<u8>(OcclusionQueryResult::Invisible)
			== bgfx::OcclusionQueryResult::Invisible,
			"OcclusionQueryResult::Invisible must mirror bgfx");
		static_assert(static_cast<u8>(OcclusionQueryResult::Visible)
			== bgfx::OcclusionQueryResult::Visible,
			"OcclusionQueryResult::Visible must mirror bgfx");
		static_assert(static_cast<u8>(OcclusionQueryResult::NoResult)
			== bgfx::OcclusionQueryResult::NoResult,
			"OcclusionQueryResult::NoResult must mirror bgfx");
	} // anonymous namespace
#endif

	OcclusionQuery::OcclusionQuery(OcclusionQuery&& other) noexcept
		: mHandle{ other.mHandle }
		, mGeneration{ other.mGeneration }
	{
		other.mHandle = InvalidHandle;
		other.mGeneration = 0;
	}

	OcclusionQuery& OcclusionQuery::operator=(OcclusionQuery&& other) noexcept
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

	OcclusionQuery::~OcclusionQuery()
	{
		reset();
	}

	OcclusionQuery OcclusionQuery::create()
	{
		OcclusionQuery result;

#if defined(WGFX_BACKEND_BGFX)
		const u32 generation{ detail::device_generation() };
		if (!detail::device_active() || generation == 0)
			return result;

		const bgfx::OcclusionQueryHandle handle{ bgfx::createOcclusionQuery() };
		if (!bgfx::isValid(handle))
			return result;

		result.mHandle = handle.idx;
		result.mGeneration = generation;
#endif

		return result;
	}

	bool OcclusionQuery::valid() const
	{
		return mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	bool OcclusionQuery::result(
		OcclusionQueryResult& result,
		i32* pixels) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		switch (bgfx::getResult(
			bgfx::OcclusionQueryHandle{ mHandle },
			pixels))
		{
		case bgfx::OcclusionQueryResult::Invisible:
			result = OcclusionQueryResult::Invisible;
			return true;
		case bgfx::OcclusionQueryResult::Visible:
			result = OcclusionQueryResult::Visible;
			return true;
		case bgfx::OcclusionQueryResult::NoResult:
			result = OcclusionQueryResult::NoResult;
			return true;
		default:
			break;
		}

		result = OcclusionQueryResult::NoResult;
		return false;
#else
		(void)result;
		(void)pixels;
		return false;
#endif
	}

	bool OcclusionQuery::setCondition(bool visible) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid())
			return false;

		bgfx::setCondition(
			bgfx::OcclusionQueryHandle{ mHandle },
			visible);
		return true;
#else
		(void)visible;
		return false;
#endif
	}

	void OcclusionQuery::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mHandle != InvalidHandle
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::destroy(bgfx::OcclusionQueryHandle{ mHandle });
		}
#endif

		mHandle = InvalidHandle;
		mGeneration = 0;
	}
}
