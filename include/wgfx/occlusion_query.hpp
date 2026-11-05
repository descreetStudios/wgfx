#pragma once

#include <wcore/types.hpp>

namespace wgfx
{
	namespace detail
	{
		struct OcclusionQueryAccess;
	}

	enum class OcclusionQueryResult : u8
	{
		Invisible,
		Visible,
		NoResult,
	};

	class OcclusionQuery
	{
	public:
		OcclusionQuery() = default;
		~OcclusionQuery();

		OcclusionQuery(const OcclusionQuery&) = delete;
		OcclusionQuery& operator=(const OcclusionQuery&) = delete;

		OcclusionQuery(OcclusionQuery&& other) noexcept;
		OcclusionQuery& operator=(OcclusionQuery&& other) noexcept;

		static OcclusionQuery create();

		bool valid() const;
		void reset();

		bool result(OcclusionQueryResult& result, i32* pixels = nullptr) const;
		bool setCondition(bool visible) const;

	private:
		friend struct detail::OcclusionQueryAccess;

		static constexpr u16 InvalidHandle{ 0xffff };

		u16 mHandle{ InvalidHandle };
		u32 mGeneration{ 0 };
	};
}
