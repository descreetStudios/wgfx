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

TEST(uniform_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Uniform color{
		wgfx::Uniform::create("u_testColor", wgfx::UniformType::Vec4) };
	const wgfx::Uniform texture{
		wgfx::Uniform::create("s_testTexture", wgfx::UniformType::Sampler) };

	CHECK(color.valid());
	CHECK(texture.valid());
}

TEST(uniform_sets_values)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Uniform color{
		wgfx::Uniform::create("u_testSetColor", wgfx::UniformType::Vec4) };
	REQUIRE(color.valid());

	const float value[4]{ 1.0f, 0.5f, 0.25f, 1.0f };
	CHECK(color.set(value));
	CHECK(!color.set(nullptr));
	CHECK(!color.set(value, 0));

	const wgfx::Uniform uninitialized;
	CHECK(!uninitialized.set(value));
}

TEST(uniform_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(!wgfx::Uniform::create(nullptr, wgfx::UniformType::Vec4).valid());
	CHECK(!wgfx::Uniform::create("", wgfx::UniformType::Vec4).valid());
	CHECK(!wgfx::Uniform::create("u_testZeroCount", wgfx::UniformType::Vec4, 0).valid());
	CHECK(!wgfx::Uniform::create(
		"u_testInvalidType",
		static_cast<wgfx::UniformType>(wgfx::UniformType::Count)).valid());
	CHECK(!wgfx::Uniform::create("1invalidName", wgfx::UniformType::Vec4).valid());
	CHECK(!wgfx::Uniform::create("u_view", wgfx::UniformType::Vec4).valid());
}

TEST(uniform_requires_active_device)
{
	CHECK(!wgfx::Uniform::create("u_testNoDevice", wgfx::UniformType::Vec4).valid());

	const float value[4]{};
	const wgfx::Uniform uniform;
	CHECK(!uniform.set(value));
}

TEST(uniform_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::Uniform::create("u_testMoveFirst", wgfx::UniformType::Vec4);
	auto second = wgfx::Uniform::create("u_testMoveSecond", wgfx::UniformType::Mat4);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::Uniform moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(uniform_duplicate_references_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::Uniform::create("u_testDuplicate", wgfx::UniformType::Vec4);
	auto second = wgfx::Uniform::create("u_testDuplicate", wgfx::UniformType::Vec4);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(uniform_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto uniform = wgfx::Uniform::create("u_testDeviceLifetime", wgfx::UniformType::Vec4);
	REQUIRE(uniform.valid());

	const float value[4]{};
	device->shutdown();
	CHECK(!uniform.valid());
	CHECK(!uniform.set(value));
	CHECK(!wgfx::Uniform::create(
		"u_testDeviceLifetime", wgfx::UniformType::Vec4).valid());

	// The stale uniform is left with an old generation; reset must be a no-op.
	uniform.reset();
	CHECK(!uniform.valid());
}

TEST(uniform_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto uniform = wgfx::Uniform::create("u_testRecreate", wgfx::UniformType::Vec4);
	REQUIRE(uniform.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!uniform.valid());
}

#else

TEST(uniform_requires_bgfx_backend)
{
	wgfx::Uniform uniform{
		wgfx::Uniform::create("u_testNoBackend", wgfx::UniformType::Vec4) };

	const float value[4]{};
	CHECK(!uniform.valid());
	CHECK(!uniform.set(value));
	uniform.reset();
}

#endif
