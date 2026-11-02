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

	const u8 ProgramVertexShader[]
	{
		'V', 'S', 'H', 12,
		1, 0, 0, 0,  // hash in
		42, 0, 0, 0, // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const u8 ProgramFragmentShader[]
	{
		'F', 'S', 'H', 12,
		42, 0, 0, 0, // hash in
		2, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const u8 MismatchedFragmentShader[]
	{
		'F', 'S', 'H', 12,
		43, 0, 0, 0, // hash in
		2, 0, 0, 0,  // hash out
		0, 0, 0, 0,  // raw SRV bindings
		0, 0, 0, 0,  // raw UAV bindings
		0, 0,        // uniform count
	};

	const u8 ProgramComputeShader[]
	{
		'C', 'S', 'H', 12,
		3, 0, 0, 0, // hash in
		3, 0, 0, 0, // hash out
		0, 0, 0, 0, // raw SRV bindings
		0, 0, 0, 0, // raw UAV bindings
		0, 0,       // uniform count
	};
} // anonymous namespace

TEST(program_creates_graphics_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());

	const wgfx::Program program{ wgfx::Program::create(vertex, fragment) };
	CHECK(program.valid());
}

TEST(program_creates_compute_with_active_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader compute{
		wgfx::Shader::create(ProgramComputeShader, sizeof(ProgramComputeShader)) };
	REQUIRE(compute.valid());

	const wgfx::Program program{ wgfx::Program::create_compute(compute) };
	CHECK(program.valid());
}

TEST(program_rejects_invalid_input)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	const wgfx::Shader mismatched{
		wgfx::Shader::create(MismatchedFragmentShader, sizeof(MismatchedFragmentShader)) };
	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());
	REQUIRE(mismatched.valid());

	CHECK(!wgfx::Program::create(vertex, mismatched).valid());
	CHECK(!wgfx::Program::create(wgfx::Shader{}, fragment).valid());
	CHECK(!wgfx::Program::create(vertex, wgfx::Shader{}).valid());
	CHECK(!wgfx::Program::create_compute(wgfx::Shader{}).valid());
}

TEST(program_requires_active_device)
{
	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	const wgfx::Shader compute{
		wgfx::Shader::create(ProgramComputeShader, sizeof(ProgramComputeShader)) };

	CHECK(!wgfx::Program::create(vertex, fragment).valid());
	CHECK(!wgfx::Program::create_compute(compute).valid());
}

TEST(program_is_move_only)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());

	auto first = wgfx::Program::create(vertex, fragment);
	auto second = wgfx::Program::create(vertex, fragment);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	wgfx::Program moved{ std::move(first) };
	CHECK(!first.valid());
	CHECK(moved.valid());

	second = std::move(moved);
	CHECK(!moved.valid());
	CHECK(second.valid());

	second.reset();
	CHECK(!second.valid());
	second.reset();
}

TEST(program_duplicate_references_are_independent)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());

	auto first = wgfx::Program::create(vertex, fragment);
	auto second = wgfx::Program::create(vertex, fragment);
	REQUIRE(first.valid());
	REQUIRE(second.valid());

	first.reset();
	CHECK(!first.valid());
	CHECK(second.valid());
}

TEST(program_outlives_shaders)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	wgfx::Program program;

	{
		const wgfx::Shader vertex{
			wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
		const wgfx::Shader fragment{
			wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
		REQUIRE(vertex.valid());
		REQUIRE(fragment.valid());

		program = wgfx::Program::create(vertex, fragment);
		REQUIRE(program.valid());
	}

	CHECK(program.valid());
}

TEST(program_dies_with_device)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());

	auto program = wgfx::Program::create(vertex, fragment);
	REQUIRE(program.valid());

	device->shutdown();
	CHECK(!program.valid());
	CHECK(!wgfx::Program::create(vertex, fragment).valid());

	// The stale program is left with an old generation; reset must be a no-op.
	program.reset();
	CHECK(!program.valid());
}

TEST(program_handle_is_stale_after_recreate)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const wgfx::Shader vertex{
		wgfx::Shader::create(ProgramVertexShader, sizeof(ProgramVertexShader)) };
	const wgfx::Shader fragment{
		wgfx::Shader::create(ProgramFragmentShader, sizeof(ProgramFragmentShader)) };
	REQUIRE(vertex.valid());
	REQUIRE(fragment.valid());

	auto program = wgfx::Program::create(vertex, fragment);
	REQUIRE(program.valid());

	device->shutdown();

	auto recreated = wgfx::Device::create(headless_desc());
	REQUIRE(recreated != nullptr);
	CHECK(!program.valid());
}

#else

TEST(program_requires_bgfx_backend)
{
	const wgfx::Shader vertex;
	const wgfx::Shader fragment;
	const wgfx::Shader compute;

	wgfx::Program graphics{ wgfx::Program::create(vertex, fragment) };
	wgfx::Program computeProgram{ wgfx::Program::create_compute(compute) };

	CHECK(!graphics.valid());
	CHECK(!computeProgram.valid());

	graphics.reset();
	computeProgram.reset();
}

#endif
