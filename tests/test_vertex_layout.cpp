#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <utility>

using namespace wcore;
using namespace test;

#if defined(WGFX_BACKEND_BGFX)

namespace
{
	wgfx::VertexLayout triangle_layout()
	{
		wgfx::VertexLayout layout;
		layout.begin()
			.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float)
			.add(wgfx::VertexAttribute::Normal, 3, wgfx::VertexAttributeType::Float)
			.add(wgfx::VertexAttribute::TexCoord0, 2, wgfx::VertexAttributeType::Float)
			.end();
		return layout;
	}
} // anonymous namespace

TEST(vertex_layout_builds_offsets_and_stride)
{
	const wgfx::VertexLayout layout{ triangle_layout() };

	REQUIRE(layout.valid());
	CHECK_EQ(layout.stride(), 32u);
	CHECK_EQ(layout.offset(wgfx::VertexAttribute::Position), 0u);
	CHECK_EQ(layout.offset(wgfx::VertexAttribute::Normal), 12u);
	CHECK_EQ(layout.offset(wgfx::VertexAttribute::TexCoord0), 24u);
	CHECK_EQ(layout.size(10), 320u);
	CHECK(layout.has(wgfx::VertexAttribute::Position));
	CHECK(layout.has(wgfx::VertexAttribute::TexCoord0));
	CHECK(!layout.has(wgfx::VertexAttribute::Tangent));
}

TEST(vertex_layout_defaults_to_empty)
{
	const wgfx::VertexLayout layout;

	CHECK(!layout.valid());
	CHECK_EQ(layout.stride(), 0u);
	CHECK_EQ(layout.hash(), 0u);
	CHECK(!layout.has(wgfx::VertexAttribute::Position));
}

TEST(vertex_layout_begin_resets_state)
{
	wgfx::VertexLayout layout{ triangle_layout() };

	layout.begin();
	CHECK(!layout.valid());
	CHECK_EQ(layout.stride(), 0u);
	CHECK(!layout.has(wgfx::VertexAttribute::Position));

	layout.end();
	CHECK(layout.valid());
	CHECK_EQ(layout.stride(), 0u);
}

TEST(vertex_layout_skip_adds_padding)
{
	wgfx::VertexLayout layout;
	layout.begin()
		.add(wgfx::VertexAttribute::Color0, 4, wgfx::VertexAttributeType::Uint8, true)
		.skip(2)
		.add(wgfx::VertexAttribute::TexCoord0, 2, wgfx::VertexAttributeType::Half)
		.end();

	CHECK_EQ(layout.stride(), 10u);
	CHECK_EQ(layout.offset(wgfx::VertexAttribute::TexCoord0), 6u);
	CHECK_EQ(layout.size(2), 20u);
}

TEST(vertex_layout_uses_renderer_attribute_sizes)
{
	wgfx::VertexLayout layout;
	layout.begin(wgfx::Renderer::OpenGL)
		.add(wgfx::VertexAttribute::TexCoord0, 3, wgfx::VertexAttributeType::Int16)
		.end();

	// OpenGL packs three Int16 components into 6 bytes.
	CHECK_EQ(layout.stride(), 6u);
}

TEST(vertex_layout_hash_is_stable)
{
	const wgfx::VertexLayout first{ triangle_layout() };
	const wgfx::VertexLayout second{ triangle_layout() };

	CHECK_EQ(first.hash(), second.hash());
	CHECK(first.hash() != 0u);

	wgfx::VertexLayout different;
	different.begin()
		.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float)
		.end();

	CHECK(first.hash() != different.hash());
}

TEST(vertex_layout_is_a_copyable_value)
{
	const wgfx::VertexLayout original{ triangle_layout() };

	wgfx::VertexLayout copy{ original };
	CHECK(copy.valid());
	CHECK_EQ(copy.stride(), original.stride());
	CHECK_EQ(copy.hash(), original.hash());
	CHECK(copy.has(wgfx::VertexAttribute::Position));

	wgfx::VertexLayout assigned;
	assigned = original;
	CHECK_EQ(assigned.hash(), original.hash());

	wgfx::VertexLayout moved{ std::move(copy) };
	CHECK_EQ(moved.hash(), original.hash());
}

#else

TEST(vertex_layout_requires_bgfx_backend)
{
	wgfx::VertexLayout layout;
	layout.begin()
		.add(wgfx::VertexAttribute::Position, 3, wgfx::VertexAttributeType::Float)
		.end();

	CHECK(!layout.valid());
	CHECK_EQ(layout.stride(), 0u);
	CHECK_EQ(layout.hash(), 0u);
}

#endif
