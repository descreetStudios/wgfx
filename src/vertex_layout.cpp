#include <wgfx/vertex_layout.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <wcore/assert.hpp>

#include <bgfx/bgfx.h>

#include <new>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		static_assert(static_cast<u8>(VertexAttribute::Count) == bgfx::Attrib::Count,
			"VertexAttribute must mirror bgfx::Attrib");
		static_assert(static_cast<u8>(VertexAttributeType::Count) == bgfx::AttribType::Count,
			"VertexAttributeType must mirror bgfx::AttribType");

		bgfx::VertexLayout& native(void* storage)
		{
			return *std::launder(reinterpret_cast<bgfx::VertexLayout*>(storage));
		}

		const bgfx::VertexLayout& native(const void* storage)
		{
			return *std::launder(reinterpret_cast<const bgfx::VertexLayout*>(storage));
		}

		bgfx::RendererType::Enum to_bgfx(Renderer renderer)
		{
			switch (renderer)
			{
			case Renderer::Noop: return bgfx::RendererType::Noop;
			case Renderer::Direct3D11: return bgfx::RendererType::Direct3D11;
			case Renderer::Direct3D12: return bgfx::RendererType::Direct3D12;
			case Renderer::Metal: return bgfx::RendererType::Metal;
			case Renderer::OpenGL: return bgfx::RendererType::OpenGL;
			case Renderer::OpenGLES: return bgfx::RendererType::OpenGLES;
			case Renderer::Vulkan: return bgfx::RendererType::Vulkan;
			case Renderer::WebGPU: return bgfx::RendererType::WebGPU;
			case Renderer::Auto: break;
			}

			return bgfx::RendererType::Noop;
		}

		bgfx::Attrib::Enum to_bgfx(VertexAttribute attribute)
		{
			switch (attribute)
			{
			case VertexAttribute::Position: return bgfx::Attrib::Position;
			case VertexAttribute::Normal: return bgfx::Attrib::Normal;
			case VertexAttribute::Tangent: return bgfx::Attrib::Tangent;
			case VertexAttribute::Bitangent: return bgfx::Attrib::Bitangent;
			case VertexAttribute::Color0: return bgfx::Attrib::Color0;
			case VertexAttribute::Color1: return bgfx::Attrib::Color1;
			case VertexAttribute::Color2: return bgfx::Attrib::Color2;
			case VertexAttribute::Color3: return bgfx::Attrib::Color3;
			case VertexAttribute::Indices: return bgfx::Attrib::Indices;
			case VertexAttribute::Weight: return bgfx::Attrib::Weight;
			case VertexAttribute::TexCoord0: return bgfx::Attrib::TexCoord0;
			case VertexAttribute::TexCoord1: return bgfx::Attrib::TexCoord1;
			case VertexAttribute::TexCoord2: return bgfx::Attrib::TexCoord2;
			case VertexAttribute::TexCoord3: return bgfx::Attrib::TexCoord3;
			case VertexAttribute::TexCoord4: return bgfx::Attrib::TexCoord4;
			case VertexAttribute::TexCoord5: return bgfx::Attrib::TexCoord5;
			case VertexAttribute::TexCoord6: return bgfx::Attrib::TexCoord6;
			case VertexAttribute::TexCoord7: return bgfx::Attrib::TexCoord7;
			case VertexAttribute::TexCoord8: return bgfx::Attrib::TexCoord8;
			case VertexAttribute::TexCoord9: return bgfx::Attrib::TexCoord9;
			case VertexAttribute::TexCoord10: return bgfx::Attrib::TexCoord10;
			case VertexAttribute::TexCoord11: return bgfx::Attrib::TexCoord11;
			case VertexAttribute::TexCoord12: return bgfx::Attrib::TexCoord12;
			case VertexAttribute::TexCoord13: return bgfx::Attrib::TexCoord13;
			case VertexAttribute::TexCoord14: return bgfx::Attrib::TexCoord14;
			case VertexAttribute::TexCoord15: return bgfx::Attrib::TexCoord15;
			case VertexAttribute::Count: break;
			}

			return bgfx::Attrib::Count;
		}

		bgfx::AttribType::Enum to_bgfx(VertexAttributeType type)
		{
			switch (type)
			{
			case VertexAttributeType::Int8: return bgfx::AttribType::Int8;
			case VertexAttributeType::Uint8: return bgfx::AttribType::Uint8;
			case VertexAttributeType::Uint10: return bgfx::AttribType::Uint10;
			case VertexAttributeType::Int16: return bgfx::AttribType::Int16;
			case VertexAttributeType::Uint16: return bgfx::AttribType::Uint16;
			case VertexAttributeType::Half: return bgfx::AttribType::Half;
			case VertexAttributeType::Float: return bgfx::AttribType::Float;
			case VertexAttributeType::Int32: return bgfx::AttribType::Int32;
			case VertexAttributeType::Uint32: return bgfx::AttribType::Uint32;
			case VertexAttributeType::Count: break;
			}

			return bgfx::AttribType::Count;
		}
	} // anonymous namespace
#endif

	VertexLayout::VertexLayout()
	{
#if defined(WGFX_BACKEND_BGFX)
		static_assert(sizeof(bgfx::VertexLayout) <= StorageSize,
			"VertexLayout storage is too small for bgfx::VertexLayout");
		static_assert(alignof(bgfx::VertexLayout) <= alignof(VertexLayout),
			"VertexLayout storage alignment is too small for bgfx::VertexLayout");

		new (mStorage) bgfx::VertexLayout();
		native(mStorage).begin();
#endif
	}

	VertexLayout::VertexLayout(const VertexLayout& other)
		: VertexLayout()
	{
#if defined(WGFX_BACKEND_BGFX)
		native(mStorage) = native(other.mStorage);
		mEnded = other.mEnded;
#else
		(void)other;
#endif
	}

	VertexLayout::VertexLayout(VertexLayout&& other) noexcept
		: VertexLayout(other)
	{
	}

	VertexLayout& VertexLayout::operator=(const VertexLayout& other)
	{
		if (this == &other)
			return *this;

#if defined(WGFX_BACKEND_BGFX)
		native(mStorage) = native(other.mStorage);
		mEnded = other.mEnded;
#else
		(void)other;
#endif
		return *this;
	}

	VertexLayout& VertexLayout::operator=(VertexLayout&& other) noexcept
	{
		return *this = other;
	}

	VertexLayout::~VertexLayout()
	{
#if defined(WGFX_BACKEND_BGFX)
		native(mStorage).~VertexLayout();
#endif
	}

	VertexLayout& VertexLayout::begin(Renderer renderer)
	{
#if defined(WGFX_BACKEND_BGFX)
		native(mStorage).begin(to_bgfx(renderer));
		mEnded = false;
#else
		(void)renderer;
#endif
		return *this;
	}

	VertexLayout& VertexLayout::add(
		VertexAttribute attribute,
		u8 count,
		VertexAttributeType type,
		bool normalized,
		bool asInt)
	{
#if defined(WGFX_BACKEND_BGFX)
		WCORE_ASSERT(attribute < VertexAttribute::Count,
			"Invalid vertex attribute");
		WCORE_ASSERT(type < VertexAttributeType::Count,
			"Invalid vertex attribute type");
		WCORE_ASSERT(count >= 1 && count <= 4,
			"Vertex attribute count must be between 1 and 4");

		native(mStorage).add(
			to_bgfx(attribute),
			count,
			to_bgfx(type),
			normalized,
			asInt);
#else
		(void)attribute;
		(void)count;
		(void)type;
		(void)normalized;
		(void)asInt;
#endif
		return *this;
	}

	VertexLayout& VertexLayout::skip(u8 bytes)
	{
#if defined(WGFX_BACKEND_BGFX)
		native(mStorage).skip(bytes);
#else
		(void)bytes;
#endif
		return *this;
	}

	void VertexLayout::end()
	{
#if defined(WGFX_BACKEND_BGFX)
		native(mStorage).end();
		mEnded = true;
#endif
	}

	bool VertexLayout::valid() const
	{
		return mEnded;
	}

	bool VertexLayout::has(VertexAttribute attribute) const
	{
#if defined(WGFX_BACKEND_BGFX)
		WCORE_ASSERT(attribute < VertexAttribute::Count,
			"Invalid vertex attribute");
		return native(mStorage).has(to_bgfx(attribute));
#else
		(void)attribute;
		return false;
#endif
	}

	u16 VertexLayout::offset(VertexAttribute attribute) const
	{
#if defined(WGFX_BACKEND_BGFX)
		WCORE_ASSERT(attribute < VertexAttribute::Count,
			"Invalid vertex attribute");
		return native(mStorage).getOffset(to_bgfx(attribute));
#else
		(void)attribute;
		return 0;
#endif
	}

	u16 VertexLayout::stride() const
	{
#if defined(WGFX_BACKEND_BGFX)
		return native(mStorage).getStride();
#else
		return 0;
#endif
	}

	u32 VertexLayout::size(u32 vertexCount) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return native(mStorage).getSize(vertexCount);
#else
		(void)vertexCount;
		return 0;
#endif
	}

	u32 VertexLayout::hash() const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mEnded)
			return 0;

		return native(mStorage).m_hash;
#else
		return 0;
#endif
	}
}
