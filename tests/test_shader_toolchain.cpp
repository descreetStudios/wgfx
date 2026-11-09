#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <cstring>
#include <filesystem>

using namespace wcore;
using namespace test;

#if defined(WGFX_BACKEND_BGFX) && defined(WGFX_TEST_SHADER_DIR)

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

	std::filesystem::path shader_path(const char* name)
	{
		return std::filesystem::path{ WGFX_TEST_SHADER_DIR } / name;
	}
} // anonymous namespace

TEST(toolchain_loads_compiled_shaders)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto vertex = wgfx::Shader::load(shader_path("vs_test.bin"));
	auto fragment = wgfx::Shader::load(shader_path("fs_test.bin"));

	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());

	const auto program = wgfx::Program::create(vertex, fragment);
	CHECK(program.valid());
}

TEST(toolchain_reports_reflected_uniforms)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto fragment = wgfx::Shader::load(shader_path("fs_test.bin"));
	REQUIRE(fragment.valid());

	REQUIRE(fragment.uniforms() == 1);

	wgfx::ShaderUniformInfo info{};
	CHECK(fragment.uniforms(&info, 1) == 1);
	CHECK(std::strcmp(info.name, "u_color") == 0);
	CHECK(info.type == wgfx::UniformType::Vec4);
	CHECK(info.count == 1);
}

TEST(toolchain_dispatches_compiled_compute_shader)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	auto compute = wgfx::Shader::load(shader_path("cs_test.bin"));
	REQUIRE(compute.valid());

	const auto program = wgfx::Program::create_compute(compute);
	REQUIRE(program.valid());

	REQUIRE(compute.uniforms() == 1);
	wgfx::ShaderUniformInfo info{};
	CHECK(compute.uniforms(&info, 1) == 1);
	CHECK(std::strcmp(info.name, "u_color") == 0);
	CHECK(info.type == wgfx::UniformType::Vec4);

	wgfx::TextureDesc desc;
	desc.width = 8;
	desc.height = 8;
	desc.format = wgfx::TextureFormat::RGBA8;
	desc.flags = wgfx::TextureFlags::ComputeWrite;
	const wgfx::Texture image{ wgfx::Texture::create(desc) };
	const wgfx::IndirectBuffer indirect{ wgfx::IndirectBuffer::create(1) };
	REQUIRE(image.valid());
	REQUIRE(indirect.valid());

	const wgfx::View view{ wgfx::View::create(0) };
	REQUIRE(view.valid());

	CHECK(view.set_image(0, image, 0, wgfx::Access::Write));
	CHECK(view.dispatch(program, 1, 1, 1));
	CHECK(view.dispatch(program, indirect));
}

TEST(toolchain_missing_shader_file_fails)
{
	const auto shader = wgfx::Shader::load(shader_path("does_not_exist.bin"));
	CHECK(!shader.valid());
}

#endif
