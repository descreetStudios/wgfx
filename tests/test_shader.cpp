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

	const u8 MinimalVertexShader[]
	{
		'V', 'S', 'H', 12,
		1, 0, 0, 0, // hash in
		1, 0, 0, 0, // hash out
		0, 0, 0, 0, // raw SRV bindings
		0, 0, 0, 0, // raw UAV bindings
		0, 0,       // uniform count
	};

	const u8 MinimalFragmentShader[]
	{
		'F', 'S', 'H', 12,
		2, 0, 0, 0, // hash in
		2, 0, 0, 0, // hash out
		0, 0, 0, 0, // raw SRV bindings
		0, 0, 0, 0, // raw UAV bindings
		0, 0,       // uniform count
	};
} // anonymous namespace

TEST(shader_creates_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(MinimalFragmentShader, sizeof(MinimalFragmentShader)) };

	CHECK(vertex.valid());
	CHECK(fragment.valid());
}

TEST(shader_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	CHECK(!wgfx::Shader::create(nullptr, 0).valid());
	CHECK(!wgfx::Shader::create(MinimalVertexShader, 0).valid());

	const u8 garbage[8]{};
	CHECK(!wgfx::Shader::create(garbage, sizeof(garbage)).valid());

	const u8 truncated[]{ 'V', 'S', 'H', 12, 0, 0, 0, 0 };
	CHECK(!wgfx::Shader::create(truncated, sizeof(truncated)).valid());
}

TEST(shader_requires_active_device)
{
	CHECK(!wgfx::Shader::create(
		MinimalVertexShader, sizeof(MinimalVertexShader)).valid());
}

TEST(shader_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader));
	auto second = wgfx::Shader::create(MinimalFragmentShader, sizeof(MinimalFragmentShader));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::Shader moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(shader_duplicate_references_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto first = wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader));
	auto second = wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader));
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(shader_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto shader = wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader));
	REQUIRE(shader.valid());

	device->shutdown();
	CHECK(!shader.valid());
	CHECK(!wgfx::Shader::create(
		MinimalVertexShader, sizeof(MinimalVertexShader)).valid());

	// The stale shader is left with an old generation; reset must be a no-op.
	shader.reset();
	CHECK(!shader.valid());
}

TEST(shader_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto shader = wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader));
	REQUIRE(shader.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!shader.valid());
}

#else

TEST(shader_requires_bgfx_backend)
{
	const u8 shaderData[]{ 'V', 'S', 'H', 12 };
	wgfx::Shader shader{ wgfx::Shader::create(shaderData, sizeof(shaderData)) };

	CHECK(!shader.valid());
	shader.reset();
}

#endif
