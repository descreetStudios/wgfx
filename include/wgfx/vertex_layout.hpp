#pragma once

#include <wgfx/device.hpp>

#include <cstddef>

namespace wgfx
{
	enum class VertexAttribute : u8
	{
		Position,
		Normal,
		Tangent,
		Bitangent,
		Color0,
		Color1,
		Color2,
		Color3,
		Indices,
		Weight,
		TexCoord0,
		TexCoord1,
		TexCoord2,
		TexCoord3,
		TexCoord4,
		TexCoord5,
		TexCoord6,
		TexCoord7,
		TexCoord8,
		TexCoord9,
		TexCoord10,
		TexCoord11,
		TexCoord12,
		TexCoord13,
		TexCoord14,
		TexCoord15,

		Count,
	};

	enum class VertexAttributeType : u8
	{
		Int8,
		Uint8,
		Uint10,
		Int16,
		Uint16,
		Half,
		Float,
		Int32,
		Uint32,

		Count,
	};

	class VertexLayout
	{
	public:
		VertexLayout();
		VertexLayout(const VertexLayout& other);
		VertexLayout(VertexLayout&& other) noexcept;
		VertexLayout& operator=(const VertexLayout& other);
		VertexLayout& operator=(VertexLayout&& other) noexcept;
		~VertexLayout();

		VertexLayout& begin(Renderer renderer = Renderer::Auto);

		VertexLayout& add(
			VertexAttribute attribute,
			u8 count,
			VertexAttributeType type,
			bool normalized = false,
			bool asInt = false);

		VertexLayout& skip(u8 bytes);
		void end();
		bool valid() const;

		bool has(VertexAttribute attribute) const;
		u16 offset(VertexAttribute attribute) const;
		u16 stride() const;
		u32 size(u32 vertexCount) const;

		u32 hash() const;

	private:
		static constexpr std::size_t StorageSize{ 128 };

		alignas(16) unsigned char mStorage[StorageSize]{};
		bool mEnded{ false };
	};
} // namespace wgfx
