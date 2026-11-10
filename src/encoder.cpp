#include <wgfx/encoder.hpp>
#include <wgfx/view.hpp>

#include "device_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "draw_common.hpp"

#include <bgfx/bgfx.h>

#include <limits>
#endif

namespace wgfx
{
#if defined(WGFX_BACKEND_BGFX)
	namespace
	{
		bgfx::Encoder* native(void* encoder)
		{
			return static_cast<bgfx::Encoder*>(encoder);
		}

		detail::draw::EncoderTarget target(void* encoder)
		{
			return detail::draw::EncoderTarget{ native(encoder) };
		}
	} // anonymous namespace
#endif

	Encoder::~Encoder()
	{
		reset();
	}

	Encoder::Encoder(Encoder&& other) noexcept
		: mEncoder{ other.mEncoder }
		, mGeneration{ other.mGeneration }
	{
		other.mEncoder = nullptr;
		other.mGeneration = 0;
	}

	Encoder& Encoder::operator=(Encoder&& other) noexcept
	{
		if (this != &other)
		{
			reset();
			mEncoder = other.mEncoder;
			mGeneration = other.mGeneration;
			other.mEncoder = nullptr;
			other.mGeneration = 0;
		}

		return *this;
	}

	Encoder Encoder::create(bool forceNewEncoder)
	{
		Encoder result;

#if defined(WGFX_BACKEND_BGFX)
		if (!detail::device_active())
			return result;

		const u32 generation{ detail::device_generation() };
		if (generation == 0)
			return result;

		bgfx::Encoder* encoder{ bgfx::begin(forceNewEncoder) };
		if (!encoder)
			return result;

		result.mEncoder = encoder;
		result.mGeneration = generation;
#else
		(void)forceNewEncoder;
#endif

		return result;
	}

	bool Encoder::valid() const
	{
		return mEncoder != nullptr
			&& detail::device_active()
			&& mGeneration == detail::device_generation();
	}

	void Encoder::reset()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (mEncoder
			&& detail::device_active()
			&& mGeneration == detail::device_generation())
		{
			bgfx::end(native(mEncoder));
		}
#endif

		mEncoder = nullptr;
		mGeneration = 0;
	}

	bool Encoder::set_marker(const char* name, i32 length) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !name)
			return false;

		native(mEncoder)->setMarker(
			name,
			length < 0 ? std::numeric_limits<i32>::max() : length);
		return true;
#else
		(void)name;
		(void)length;
		return false;
#endif
	}

	bool Encoder::set_condition(
		const OcclusionQuery& query,
		bool visible) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!valid() || !query.valid())
			return false;

		native(mEncoder)->setCondition(
			detail::OcclusionQueryAccess::native(query),
			visible);
		return true;
#else
		(void)query;
		(void)visible;
		return false;
#endif
	}

	bool Encoder::set_state(const RenderState& state) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_state(target(mEncoder), valid(), state);
#else
		(void)state;
		return false;
#endif
	}

	bool Encoder::set_state(RenderStateFlags flags, u32 rgba) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_state(target(mEncoder), valid(), flags, rgba);
#else
		(void)flags;
		(void)rgba;
		return false;
#endif
	}

	bool Encoder::set_vertex_count(u32 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_count(
			target(mEncoder), valid(), count);
#else
		(void)count;
		return false;
#endif
	}

	bool Encoder::set_model_transform(const void* matrix, u16 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_model_transform(
			target(mEncoder), valid(), matrix, count);
#else
		(void)matrix;
		(void)count;
		return false;
#endif
	}

	bool Encoder::set_draw_scissor(
		u16 x, u16 y,
		u16 width,
		u16 height) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_draw_scissor(
			target(mEncoder), valid(), x, y, width, height);
#else
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		return false;
#endif
	}

	bool Encoder::set_depth_control(
		i32 constant,
		float slopeScale,
		float clamp,
		bool depthClamp) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_depth_control(
			target(mEncoder), valid(),
			constant, slopeScale, clamp, depthClamp);
#else
		(void)constant;
		(void)slopeScale;
		(void)clamp;
		(void)depthClamp;
		return false;
#endif
	}

	bool Encoder::set_sample_mask(u32 mask) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_sample_mask(
			target(mEncoder), valid(), mask);
#else
		(void)mask;
		return false;
#endif
	}

	bool Encoder::set_stencil(u32 front, u32 back) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_stencil(
			target(mEncoder), valid(), front, back);
#else
		(void)front;
		(void)back;
		return false;
#endif
	}

	bool Encoder::set_vertex_buffer(
		u8 stream,
		const VertexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			target(mEncoder), valid(), stream, buffer);
#else
		(void)stream;
		(void)buffer;
		return false;
#endif
	}

	bool Encoder::set_vertex_buffer(
		u8 stream,
		const VertexBuffer& buffer,
		u32 startVertex,
		u32 numVertices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			target(mEncoder), valid(),
			stream, buffer, startVertex, numVertices);
#else
		(void)stream;
		(void)buffer;
		(void)startVertex;
		(void)numVertices;
		return false;
#endif
	}

	bool Encoder::set_vertex_buffer(
		u8 stream,
		const DynamicVertexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			target(mEncoder), valid(), stream, buffer);
#else
		(void)stream;
		(void)buffer;
		return false;
#endif
	}

	bool Encoder::set_vertex_buffer(
		u8 stream,
		const DynamicVertexBuffer& buffer,
		u32 startVertex,
		u32 numVertices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			target(mEncoder), valid(),
			stream, buffer, startVertex, numVertices);
#else
		(void)stream;
		(void)buffer;
		(void)startVertex;
		(void)numVertices;
		return false;
#endif
	}

	bool Encoder::set_vertex_buffer(
		u8 stream,
		const TransientVertexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			target(mEncoder), valid(), stream, buffer);
#else
		(void)stream;
		(void)buffer;
		return false;
#endif
	}

	bool Encoder::set_vertex_buffer(
		u8 stream,
		const TransientVertexBuffer& buffer,
		u32 startVertex,
		u32 numVertices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_vertex_buffer(
			target(mEncoder), valid(),
			stream, buffer, startVertex, numVertices);
#else
		(void)stream;
		(void)buffer;
		(void)startVertex;
		(void)numVertices;
		return false;
#endif
	}

	bool Encoder::set_index_buffer(const IndexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			target(mEncoder), valid(), buffer);
#else
		(void)buffer;
		return false;
#endif
	}

	bool Encoder::set_index_buffer(
		const IndexBuffer& buffer,
		u32 firstIndex,
		u32 numIndices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			target(mEncoder), valid(), buffer, firstIndex, numIndices);
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool Encoder::set_index_buffer(const DynamicIndexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			target(mEncoder), valid(), buffer);
#else
		(void)buffer;
		return false;
#endif
	}

	bool Encoder::set_index_buffer(
		const DynamicIndexBuffer& buffer,
		u32 firstIndex,
		u32 numIndices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			target(mEncoder), valid(), buffer, firstIndex, numIndices);
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool Encoder::set_index_buffer(const TransientIndexBuffer& buffer) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			target(mEncoder), valid(), buffer);
#else
		(void)buffer;
		return false;
#endif
	}

	bool Encoder::set_index_buffer(
		const TransientIndexBuffer& buffer,
		u32 firstIndex,
		u32 numIndices) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_index_buffer(
			target(mEncoder), valid(), buffer, firstIndex, numIndices);
#else
		(void)buffer;
		(void)firstIndex;
		(void)numIndices;
		return false;
#endif
	}

	bool Encoder::set_instance_count(u32 count) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_count(
			target(mEncoder), valid(), count);
#else
		(void)count;
		return false;
#endif
	}

	bool Encoder::set_instance_data_buffer(
		const VertexBuffer& buffer,
		u32 startVertex,
		u32 numInstances) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_data_buffer(
			target(mEncoder), valid(),
			buffer, startVertex, numInstances);
#else
		(void)buffer;
		(void)startVertex;
		(void)numInstances;
		return false;
#endif
	}

	bool Encoder::set_instance_data_buffer(
		const DynamicVertexBuffer& buffer,
		u32 startVertex,
		u32 numInstances) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_data_buffer(
			target(mEncoder), valid(),
			buffer, startVertex, numInstances);
#else
		(void)buffer;
		(void)startVertex;
		(void)numInstances;
		return false;
#endif
	}

	bool Encoder::set_instance_data_buffer(
		const InstanceDataBuffer& buffer,
		u32 start,
		u32 num) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_instance_data_buffer(
			target(mEncoder), valid(), buffer, start, num);
#else
		(void)buffer;
		(void)start;
		(void)num;
		return false;
#endif
	}

	bool Encoder::set_texture(
		u8 stage,
		const Uniform& sampler,
		const Texture& texture) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_texture(
			target(mEncoder), valid(), stage, sampler, texture);
#else
		(void)stage;
		(void)sampler;
		(void)texture;
		return false;
#endif
	}

	bool Encoder::set_texture(
		u8 stage,
		const Uniform& sampler,
		const Texture& texture,
		SamplerFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_texture(
			target(mEncoder), valid(),
			stage, sampler, texture, flags);
#else
		(void)stage;
		(void)sampler;
		(void)texture;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::set_image(
		u8 stage,
		const Texture& texture,
		u8 mip,
		Access access,
		TextureFormat format) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_image(
			target(mEncoder), valid(),
			stage, texture, mip, access, format);
#else
		(void)stage;
		(void)texture;
		(void)mip;
		(void)access;
		(void)format;
		return false;
#endif
	}

	bool Encoder::set_image(
		u8 stage,
		const Texture& texture,
		u16 firstLayer,
		u16 numLayers,
		u8 mip,
		Access access,
		TextureFormat format) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_image(
			target(mEncoder), valid(),
			stage, texture, firstLayer, numLayers, mip, access, format);
#else
		(void)stage;
		(void)texture;
		(void)firstLayer;
		(void)numLayers;
		(void)mip;
		(void)access;
		(void)format;
		return false;
#endif
	}

	bool Encoder::set_buffer(
		u8 stage,
		const VertexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			target(mEncoder), valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool Encoder::set_buffer(
		u8 stage,
		const IndexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			target(mEncoder), valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool Encoder::set_buffer(
		u8 stage,
		const DynamicVertexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			target(mEncoder), valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool Encoder::set_buffer(
		u8 stage,
		const DynamicIndexBuffer& buffer,
		Access access,
		u32 offset,
		u32 size) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			target(mEncoder), valid(),
			stage, buffer, access, offset, size);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		(void)offset;
		(void)size;
		return false;
#endif
	}

	bool Encoder::set_buffer(
		u8 stage,
		const IndirectBuffer& buffer,
		Access access) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::set_buffer(
			target(mEncoder), valid(), stage, buffer, access);
#else
		(void)stage;
		(void)buffer;
		(void)access;
		return false;
#endif
	}

	bool Encoder::touch(const View& view) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::touch(
			target(mEncoder), valid() && view.valid(), view.id());
#else
		(void)view;
		return false;
#endif
	}

	bool Encoder::dispatch(
		const View& view,
		const Program& program,
		u32 numX,
		u32 numY,
		u32 numZ,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::dispatch(
			target(mEncoder), valid() && view.valid(), view.id(),
			program, numX, numY, numZ, flags);
#else
		(void)view;
		(void)program;
		(void)numX;
		(void)numY;
		(void)numZ;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::dispatch(
		const View& view,
		const Program& program,
		const IndirectBuffer& indirect,
		u32 start,
		u32 num,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::dispatch(
			target(mEncoder), valid() && view.valid(), view.id(),
			program, indirect, start, num, flags);
#else
		(void)view;
		(void)program;
		(void)indirect;
		(void)start;
		(void)num;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::submit(
		const View& view,
		const Program& program,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::submit(
			target(mEncoder), valid() && view.valid(), view.id(),
			program, depth, flags);
#else
		(void)view;
		(void)program;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::submit(
		const View& view,
		const Program& program,
		const OcclusionQuery& query,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::submit(
			target(mEncoder), valid() && view.valid(), view.id(),
			program, query, depth, flags);
#else
		(void)view;
		(void)program;
		(void)query;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::submit(
		const View& view,
		const Program& program,
		const IndirectBuffer& indirect,
		u32 start,
		u32 num,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::submit(
			target(mEncoder), valid() && view.valid(), view.id(),
			program, indirect, start, num, depth, flags);
#else
		(void)view;
		(void)program;
		(void)indirect;
		(void)start;
		(void)num;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::submit(
		const View& view,
		const Program& program,
		const IndirectBuffer& indirect,
		u32 start,
		const IndexBuffer& numBuffer,
		u32 numIndex,
		u32 numMax,
		u32 depth,
		DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::submit(
			target(mEncoder), valid() && view.valid(), view.id(),
			program, indirect, start, numBuffer, numIndex, numMax,
			depth, flags);
#else
		(void)view;
		(void)program;
		(void)indirect;
		(void)start;
		(void)numBuffer;
		(void)numIndex;
		(void)numMax;
		(void)depth;
		(void)flags;
		return false;
#endif
	}

	bool Encoder::discard(DiscardFlags flags) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::discard(target(mEncoder), valid(), flags);
#else
		(void)flags;
		return false;
#endif
	}

	bool Encoder::blit(
		const View& view,
		const TextureRegion& dst,
		const TextureRegion& src) const
	{
#if defined(WGFX_BACKEND_BGFX)
		return detail::draw::blit_texture(
			target(mEncoder), valid() && view.valid(), view.id(), dst, src);
#else
		(void)view;
		(void)dst;
		(void)src;
		return false;
#endif
	}
}
