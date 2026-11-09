#include <wcore/io.hpp>
#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <cstring>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>

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

	std::vector<u8> shader_with_uniforms()
	{
		std::vector<u8> data
		{
			'V', 'S', 'H', 12,
			1, 0, 0, 0,  // hash in
			42, 0, 0, 0, // hash out
			0, 0, 0, 0,  // raw SRV bindings
			0, 0, 0, 0,  // raw UAV bindings
			2, 0,        // uniform count
		};

		const auto add_uniform = [&data](const char* name, u8 type, u8 count)
		{
			const u8 nameSize{ static_cast<u8>(std::strlen(name)) };
			data.push_back(nameSize);
			data.insert(data.end(), name, name + nameSize);
			data.push_back(type);
			data.push_back(count);
			data.push_back(0); data.push_back(0); // register index
			data.push_back(1); data.push_back(0); // register count
			data.push_back(0); // texture component
			data.push_back(0); // texture dimension
			data.push_back(0); data.push_back(0); // texture format
		};

		add_uniform("u_color", 2 /* Vec4 */, 3);
		add_uniform("s_tex", 0 /* Sampler */, 1);
		return data;
	}
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

TEST(shader_reports_uniforms)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto data = shader_with_uniforms();
	const wgfx::Shader shader{
		wgfx::Shader::create(data.data(), static_cast<u32>(data.size())) };
	REQUIRE(shader.valid());

	CHECK(shader.uniforms() == 2);

	wgfx::ShaderUniformInfo infos[2]{};
	CHECK(shader.uniforms(infos, 2) == 2);

	CHECK(std::strcmp(infos[0].name, "u_color") == 0);
	CHECK(infos[0].type == wgfx::UniformType::Vec4);
	CHECK(infos[0].count == 3);

	CHECK(std::strcmp(infos[1].name, "s_tex") == 0);
	CHECK(infos[1].type == wgfx::UniformType::Sampler);
	CHECK(infos[1].count == 1);
}

TEST(shader_uniforms_respect_capacity)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto data = shader_with_uniforms();
	const wgfx::Shader shader{
		wgfx::Shader::create(data.data(), static_cast<u32>(data.size())) };
	REQUIRE(shader.valid());

	wgfx::ShaderUniformInfo info{};
	CHECK(shader.uniforms(&info, 1) == 2);
	CHECK(std::strcmp(info.name, "u_color") == 0);
	CHECK(info.type == wgfx::UniformType::Vec4);
	CHECK(info.count == 3);
}

TEST(shader_without_uniforms_reports_none)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader shader{
		wgfx::Shader::create(MinimalVertexShader, sizeof(MinimalVertexShader)) };
	REQUIRE(shader.valid());

	CHECK(shader.uniforms() == 0);

	wgfx::ShaderUniformInfo info{};
	CHECK(shader.uniforms(&info, 1) == 0);
}

TEST(shader_uniform_introspection_requires_valid_shader)
{
	wgfx::Shader shader;

	CHECK(shader.uniforms() == 0);

	wgfx::ShaderUniformInfo info{};
	CHECK(shader.uniforms(&info, 1) == 0);
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

TEST(shader_uniforms_die_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto data = shader_with_uniforms();
	auto shader = wgfx::Shader::create(
		data.data(),
		static_cast<u32>(data.size()));
	REQUIRE(shader.valid());
	CHECK(shader.uniforms() == 2);

	device->shutdown();
	CHECK(shader.uniforms() == 0);
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

TEST(shader_loads_compiled_binary_from_file)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const auto temp{ wcore::io::temp_directory() };
	REQUIRE(temp.has_value());
	const std::filesystem::path path{ *temp / "wgfx_shader_load_test.bin" };

	const std::span<const std::byte> bytes{
		reinterpret_cast<const std::byte*>(MinimalVertexShader),
		sizeof(MinimalVertexShader) };
	REQUIRE(wcore::io::write_binary(path, bytes));

	const auto shader{ wgfx::Shader::load(path) };
	CHECK(shader.valid());

	wcore::io::remove_file(path);
}

TEST(shader_load_missing_file_fails)
{
	const auto temp{ wcore::io::temp_directory() };
	REQUIRE(temp.has_value());

	const auto shader{ wgfx::Shader::load(*temp / "wgfx_missing_shader.bin") };
	CHECK(!shader.valid());
}

#else

TEST(shader_load_requires_bgfx_backend)
{
	const auto shader{ wgfx::Shader::load("wgfx_missing_shader.bin") };
	CHECK(!shader.valid());
}

TEST(shader_requires_bgfx_backend)
{
	const u8 shaderData[]{ 'V', 'S', 'H', 12 };
	wgfx::Shader shader{ wgfx::Shader::create(shaderData, sizeof(shaderData)) };

	CHECK(!shader.valid());
	shader.reset();
}

TEST(shader_uniforms_require_bgfx_backend)
{
	const u8 shaderData[]{ 'V', 'S', 'H', 12 };
	wgfx::Shader shader{ wgfx::Shader::create(shaderData, sizeof(shaderData)) };

	wgfx::ShaderUniformInfo info{};
	CHECK(shader.uniforms() == 0);
	CHECK(shader.uniforms(&info, 1) == 0);

	wgfx::Shader empty;
	CHECK(empty.uniforms() == 0);
	CHECK(empty.uniforms(&info, 1) == 0);
}

#endif
