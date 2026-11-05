#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <utility>

using namespace wcore;
using namespace test;

#if defined(WGFX_BACKEND_BGFX)

namespace
{
	wgfx::DeviceDesc headless_desc()
	{
		wgfx::DeviceDesc desc;
		desc.renderer = wgfx::Renderer::Noop;
		desc.width = 320;
		desc.height = 180;
		desc.vsync = false;
		return desc;
	}
} // anonymous namespace

TEST(occlusion_query_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::OcclusionQuery query{ wgfx::OcclusionQuery::create() };
	CHECK(query.valid());
}

TEST(occlusion_query_has_no_result_before_render)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::OcclusionQuery query{ wgfx::OcclusionQuery::create() };
	REQUIRE(query.valid());

	wgfx::OcclusionQueryResult result{ wgfx::OcclusionQueryResult::Visible };
	i32 pixels{ -1 };

	CHECK(query.result(result, &pixels));
	CHECK(result == wgfx::OcclusionQueryResult::NoResult);
	CHECK(pixels == -1);

	CHECK(query.result(result));
	CHECK(result == wgfx::OcclusionQueryResult::NoResult);
}

TEST(occlusion_query_sets_condition)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::OcclusionQuery query{ wgfx::OcclusionQuery::create() };
	REQUIRE(query.valid());

	CHECK(query.setCondition(true));
	CHECK(query.setCondition(false));

	const wgfx::OcclusionQuery uninitialized;
	CHECK(!uninitialized.setCondition(true));
}

TEST(occlusion_query_requires_active_device)
{
	CHECK(!wgfx::OcclusionQuery::create().valid());

	wgfx::OcclusionQueryResult result{ wgfx::OcclusionQueryResult::NoResult };
	const wgfx::OcclusionQuery query;
	CHECK(!query.result(result));
	CHECK(!query.setCondition(true));
}

TEST(occlusion_query_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::OcclusionQuery::create();
	auto second = wgfx::OcclusionQuery::create();
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::OcclusionQuery moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(occlusion_query_duplicates_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::OcclusionQuery::create();
	auto second = wgfx::OcclusionQuery::create();
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(occlusion_query_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto query = wgfx::OcclusionQuery::create();
	REQUIRE(query.valid());

	device->shutdown();
	CHECK(!query.valid());
	CHECK(!wgfx::OcclusionQuery::create().valid());

	wgfx::OcclusionQueryResult result{ wgfx::OcclusionQueryResult::NoResult };
	CHECK(!query.result(result));
	CHECK(!query.setCondition(true));

	// The stale query is left with an old generation; reset must be a no-op.
	query.reset();
	CHECK(!query.valid());
}

TEST(occlusion_query_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto query = wgfx::OcclusionQuery::create();
	REQUIRE(query.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!query.valid());
}

#else

TEST(occlusion_query_requires_bgfx_backend)
{
	wgfx::OcclusionQuery query{ wgfx::OcclusionQuery::create() };
	wgfx::OcclusionQueryResult result{ wgfx::OcclusionQueryResult::NoResult };

	CHECK(!query.valid());
	CHECK(!query.result(result));
	CHECK(!query.setCondition(true));
	query.reset();
}

#endif
