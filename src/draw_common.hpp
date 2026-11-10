#pragma once

#if defined(WGFX_BACKEND_BGFX)
#include <wgfx/access.hpp>
#include <wgfx/render_state.hpp>
#include <wgfx/texture.hpp>
#include <wgfx/transient_buffer.hpp>

#include "dynamic_index_buffer_internal.hpp"
#include "dynamic_vertex_buffer_internal.hpp"
#include "index_buffer_internal.hpp"
#include "indirect_buffer_internal.hpp"
#include "occlusion_query_internal.hpp"
#include "program_internal.hpp"
#include "texture_internal.hpp"
#include "transient_buffer_internal.hpp"
#include "uniform_internal.hpp"
#include "vertex_buffer_internal.hpp"

#include <bgfx/bgfx.h>

namespace wgfx
{
	namespace detail
	{
		namespace draw
		{
			constexpr u8 MaxVertexStreams{ 4 };
			constexpr u8 MaxStages{ 16 };
			constexpr u32 BufferOffsetAlignment{ 256 };
			constexpr u32 WholeBuffer{ 0xffffffffu };

			inline bool valid_stream(u8 stream)
			{
				return stream < MaxVertexStreams;
			}

			inline bool valid_stage(u8 stage)
			{
				return stage < MaxStages;
			}

			inline bool valid_access(Access access)
			{
				return static_cast<u8>(access)
					< static_cast<u8>(Access::Count);
			}

			inline bool valid_buffer_range(u32 offset, u32 size)
			{
				return offset % BufferOffsetAlignment == 0 && size != 0;
			}

			inline bool valid_image_format(TextureFormat format)
			{
				return format == TextureFormat::Count
					|| static_cast<u16>(format)
					< static_cast<u16>(TextureFormat::Count);
			}

			inline bool supports_draw_indirect()
			{
				const bgfx::Caps* caps{ bgfx::getCaps() };
				return caps && 0 != (caps->supported & BGFX_CAPS_DRAW_INDIRECT);
			}

			inline bool supports_draw_indirect_count()
			{
				const bgfx::Caps* caps{ bgfx::getCaps() };
				return caps
					&& 0 != (caps->supported & BGFX_CAPS_DRAW_INDIRECT_COUNT);
			}

			struct GlobalTarget
			{
				void setState(u64 state, u32 rgba)
				{
					bgfx::setState(state, rgba);
				}

				void setStencil(u32 front, u32 back)
				{
					bgfx::setStencil(front, back);
				}

				void setSampleMask(u32 mask)
				{
					bgfx::setSampleMask(mask);
				}

				void setScissor(u16 x, u16 y, u16 width, u16 height)
				{
					bgfx::setScissor(x, y, width, height);
				}

				void setDepthControl(
					i32 constant,
					float slopeScale,
					float clamp,
					bool depthClamp)
				{
					bgfx::setDepthControl(
						constant, slopeScale, clamp, depthClamp);
				}

				void setTransform(const void* matrix, u16 count)
				{
					bgfx::setTransform(matrix, count);
				}

				void setVertexCount(u32 count)
				{
					bgfx::setVertexCount(count);
				}

				void setInstanceCount(u32 count)
				{
					bgfx::setInstanceCount(count);
				}

#define WGFX_GLOBAL_FORWARD(name, ...) \
				template <typename... Args> \
				void name(Args&&... args) \
				{ \
					bgfx::name(static_cast<Args&&>(args)...); \
				}

				WGFX_GLOBAL_FORWARD(setVertexBuffer);
				WGFX_GLOBAL_FORWARD(setIndexBuffer);
				WGFX_GLOBAL_FORWARD(setInstanceDataBuffer);
				WGFX_GLOBAL_FORWARD(setTexture);
				WGFX_GLOBAL_FORWARD(setImage);
				WGFX_GLOBAL_FORWARD(setBuffer);
				WGFX_GLOBAL_FORWARD(dispatch);
				WGFX_GLOBAL_FORWARD(submit);
				WGFX_GLOBAL_FORWARD(discard);
				WGFX_GLOBAL_FORWARD(touch);
				WGFX_GLOBAL_FORWARD(blit);

#undef WGFX_GLOBAL_FORWARD
			};

			struct EncoderTarget
			{
				bgfx::Encoder* encoder;

				void setState(u64 state, u32 rgba)
				{
					encoder->setState(state, rgba);
				}

				void setStencil(u32 front, u32 back)
				{
					encoder->setStencil(front, back);
				}

				void setSampleMask(u32 mask)
				{
					encoder->setSampleMask(mask);
				}

				void setScissor(u16 x, u16 y, u16 width, u16 height)
				{
					encoder->setScissor(x, y, width, height);
				}

				void setDepthControl(
					i32 constant,
					float slopeScale,
					float clamp,
					bool depthClamp)
				{
					encoder->setDepthControl(
						constant, slopeScale, clamp, depthClamp);
				}

				void setTransform(const void* matrix, u16 count)
				{
					encoder->setTransform(matrix, count);
				}

				void setVertexCount(u32 count)
				{
					encoder->setVertexCount(count);
				}

				void setInstanceCount(u32 count)
				{
					encoder->setInstanceCount(count);
				}

#define WGFX_ENCODER_FORWARD(name) \
				template <typename... Args> \
				void name(Args&&... args) \
				{ \
					encoder->name(static_cast<Args&&>(args)...); \
				}

				WGFX_ENCODER_FORWARD(setVertexBuffer);
				WGFX_ENCODER_FORWARD(setIndexBuffer);
				WGFX_ENCODER_FORWARD(setInstanceDataBuffer);
				WGFX_ENCODER_FORWARD(setTexture);
				WGFX_ENCODER_FORWARD(setImage);
				WGFX_ENCODER_FORWARD(setBuffer);
				WGFX_ENCODER_FORWARD(dispatch);
				WGFX_ENCODER_FORWARD(submit);
				WGFX_ENCODER_FORWARD(discard);
				WGFX_ENCODER_FORWARD(touch);
				WGFX_ENCODER_FORWARD(blit);

#undef WGFX_ENCODER_FORWARD
			};

			template <typename Target>
			bool set_state(
				Target target,
				bool enabled,
				const RenderState& state)
			{
				return set_state(target, enabled, state.flags, state.rgba);
			}

			template <typename Target>
			bool set_state(
				Target target,
				bool enabled,
				RenderStateFlags flags,
				u32 rgba)
			{
				if (!enabled)
					return false;

				target.setState(static_cast<u64>(flags), rgba);
				return true;
			}

			template <typename Target>
			bool set_vertex_count(Target target, bool enabled, u32 count)
			{
				if (!enabled)
					return false;

				target.setVertexCount(count);
				return true;
			}

			template <typename Target>
			bool set_model_transform(
				Target target,
				bool enabled,
				const void* matrix,
				u16 count)
			{
				if (!enabled || !matrix || count == 0)
					return false;

				target.setTransform(matrix, count);
				return true;
			}

			template <typename Target>
			bool set_draw_scissor(
				Target target,
				bool enabled,
				u16 x,
				u16 y,
				u16 width,
				u16 height)
			{
				if (!enabled)
					return false;

				target.setScissor(x, y, width, height);
				return true;
			}

			template <typename Target>
			bool set_depth_control(
				Target target,
				bool enabled,
				i32 constant,
				float slopeScale,
				float clamp,
				bool depthClamp)
			{
				if (!enabled)
					return false;

				target.setDepthControl(
					constant, slopeScale, clamp, depthClamp);
				return true;
			}

			template <typename Target>
			bool set_sample_mask(Target target, bool enabled, u32 mask)
			{
				if (!enabled)
					return false;

				target.setSampleMask(mask);
				return true;
			}

			template <typename Target>
			bool set_stencil(
				Target target,
				bool enabled,
				u32 front,
				u32 back)
			{
				if (!enabled)
					return false;

				target.setStencil(front, back);
				return true;
			}

			template <typename Target>
			bool set_vertex_buffer(
				Target target,
				bool enabled,
				u8 stream,
				const VertexBuffer& buffer)
			{
				if (!enabled || !buffer.valid() || !valid_stream(stream))
					return false;

				target.setVertexBuffer(
					stream,
					VertexBufferAccess::native(buffer));
				return true;
			}

			template <typename Target>
			bool set_vertex_buffer(
				Target target,
				bool enabled,
				u8 stream,
				const VertexBuffer& buffer,
				u32 startVertex,
				u32 numVertices)
			{
				if (!enabled
					|| !buffer.valid()
					|| !valid_stream(stream)
					|| numVertices == 0)
				{
					return false;
				}

				target.setVertexBuffer(
					stream,
					VertexBufferAccess::native(buffer),
					startVertex,
					numVertices);
				return true;
			}

			template <typename Target>
			bool set_vertex_buffer(
				Target target,
				bool enabled,
				u8 stream,
				const DynamicVertexBuffer& buffer)
			{
				if (!enabled || !buffer.valid() || !valid_stream(stream))
					return false;

				target.setVertexBuffer(
					stream,
					DynamicVertexBufferAccess::native(buffer));
				return true;
			}

			template <typename Target>
			bool set_vertex_buffer(
				Target target,
				bool enabled,
				u8 stream,
				const DynamicVertexBuffer& buffer,
				u32 startVertex,
				u32 numVertices)
			{
				if (!enabled
					|| !buffer.valid()
					|| !valid_stream(stream)
					|| numVertices == 0)
				{
					return false;
				}

				target.setVertexBuffer(
					stream,
					DynamicVertexBufferAccess::native(buffer),
					startVertex,
					numVertices);
				return true;
			}

			template <typename Target>
			bool set_vertex_buffer(
				Target target,
				bool enabled,
				u8 stream,
				const TransientVertexBuffer& buffer)
			{
				return set_vertex_buffer(
					target, enabled, stream, buffer, 0, buffer.count());
			}

			template <typename Target>
			bool set_vertex_buffer(
				Target target,
				bool enabled,
				u8 stream,
				const TransientVertexBuffer& buffer,
				u32 startVertex,
				u32 numVertices)
			{
				if (!enabled
					|| !buffer.valid()
					|| !valid_stream(stream)
					|| numVertices == 0
					|| startVertex >= buffer.count()
					|| numVertices > buffer.count() - startVertex)
				{
					return false;
				}

				const bgfx::TransientVertexBuffer native{
					TransientBufferAccess::native(buffer) };
				target.setVertexBuffer(
					stream, &native, startVertex, numVertices);
				return true;
			}

			template <typename Target>
			bool set_index_buffer(
				Target target,
				bool enabled,
				const IndexBuffer& buffer)
			{
				if (!enabled || !buffer.valid())
					return false;

				target.setIndexBuffer(IndexBufferAccess::native(buffer));
				return true;
			}

			template <typename Target>
			bool set_index_buffer(
				Target target,
				bool enabled,
				const IndexBuffer& buffer,
				u32 firstIndex,
				u32 numIndices)
			{
				if (!enabled || !buffer.valid() || numIndices == 0)
					return false;

				target.setIndexBuffer(
					IndexBufferAccess::native(buffer),
					firstIndex,
					numIndices);
				return true;
			}

			template <typename Target>
			bool set_index_buffer(
				Target target,
				bool enabled,
				const DynamicIndexBuffer& buffer)
			{
				if (!enabled || !buffer.valid())
					return false;

				target.setIndexBuffer(
					DynamicIndexBufferAccess::native(buffer));
				return true;
			}

			template <typename Target>
			bool set_index_buffer(
				Target target,
				bool enabled,
				const DynamicIndexBuffer& buffer,
				u32 firstIndex,
				u32 numIndices)
			{
				if (!enabled || !buffer.valid() || numIndices == 0)
					return false;

				target.setIndexBuffer(
					DynamicIndexBufferAccess::native(buffer),
					firstIndex,
					numIndices);
				return true;
			}

			template <typename Target>
			bool set_index_buffer(
				Target target,
				bool enabled,
				const TransientIndexBuffer& buffer)
			{
				return set_index_buffer(
					target, enabled, buffer, 0, buffer.count());
			}

			template <typename Target>
			bool set_index_buffer(
				Target target,
				bool enabled,
				const TransientIndexBuffer& buffer,
				u32 firstIndex,
				u32 numIndices)
			{
				if (!enabled
					|| !buffer.valid()
					|| numIndices == 0
					|| firstIndex >= buffer.count()
					|| numIndices > buffer.count() - firstIndex)
				{
					return false;
				}

				const bgfx::TransientIndexBuffer native{
					TransientBufferAccess::native(buffer) };
				target.setIndexBuffer(&native, firstIndex, numIndices);
				return true;
			}

			template <typename Target>
			bool set_instance_count(Target target, bool enabled, u32 count)
			{
				if (!enabled)
					return false;

				target.setInstanceCount(count);
				return true;
			}

			template <typename Target>
			bool set_instance_data_buffer(
				Target target,
				bool enabled,
				const VertexBuffer& buffer,
				u32 startVertex,
				u32 numInstances)
			{
				if (!enabled || !buffer.valid() || numInstances == 0)
					return false;

				target.setInstanceDataBuffer(
					VertexBufferAccess::native(buffer),
					startVertex,
					numInstances);
				return true;
			}

			template <typename Target>
			bool set_instance_data_buffer(
				Target target,
				bool enabled,
				const DynamicVertexBuffer& buffer,
				u32 startVertex,
				u32 numInstances)
			{
				if (!enabled || !buffer.valid() || numInstances == 0)
					return false;

				target.setInstanceDataBuffer(
					DynamicVertexBufferAccess::native(buffer),
					startVertex,
					numInstances);
				return true;
			}

			template <typename Target>
			bool set_instance_data_buffer(
				Target target,
				bool enabled,
				const InstanceDataBuffer& buffer,
				u32 start,
				u32 num)
			{
				if (!enabled || !buffer.valid())
					return false;

				const u32 count{ buffer.count() };
				if (start >= count)
					return false;

				if (num == WholeBuffer)
					num = count - start;
				else if (num == 0 || num > count - start)
					return false;

				const bgfx::InstanceDataBuffer native{
					TransientBufferAccess::native(buffer) };
				target.setInstanceDataBuffer(&native, start, num);
				return true;
			}

			template <typename Target>
			bool set_texture(
				Target target,
				bool enabled,
				u8 stage,
				const Uniform& sampler,
				const Texture& texture)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !sampler.valid()
					|| !texture.valid())
				{
					return false;
				}

				target.setTexture(
					stage,
					UniformAccess::native(sampler),
					TextureAccess::native(texture));
				return true;
			}

			template <typename Target>
			bool set_texture(
				Target target,
				bool enabled,
				u8 stage,
				const Uniform& sampler,
				const Texture& texture,
				SamplerFlags flags)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !sampler.valid()
					|| !texture.valid())
				{
					return false;
				}

				target.setTexture(
					stage,
					UniformAccess::native(sampler),
					TextureAccess::native(texture),
					static_cast<u32>(flags));
				return true;
			}

			template <typename Target>
			bool set_image(
				Target target,
				bool enabled,
				u8 stage,
				const Texture& texture,
				u8 mip,
				Access access,
				TextureFormat format)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !texture.valid()
					|| !valid_access(access)
					|| !valid_image_format(format))
				{
					return false;
				}

				target.setImage(
					stage,
					TextureAccess::native(texture),
					mip,
					static_cast<bgfx::Access::Enum>(access),
					static_cast<bgfx::TextureFormat::Enum>(format));
				return true;
			}

			template <typename Target>
			bool set_image(
				Target target,
				bool enabled,
				u8 stage,
				const Texture& texture,
				u16 firstLayer,
				u16 numLayers,
				u8 mip,
				Access access,
				TextureFormat format)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !texture.valid()
					|| numLayers == 0
					|| !valid_access(access)
					|| !valid_image_format(format))
				{
					return false;
				}

				target.setImage(
					stage,
					TextureAccess::native(texture),
					firstLayer,
					numLayers,
					mip,
					static_cast<bgfx::Access::Enum>(access),
					static_cast<bgfx::TextureFormat::Enum>(format));
				return true;
			}

			template <typename Target>
			bool set_buffer(
				Target target,
				bool enabled,
				u8 stage,
				const VertexBuffer& buffer,
				Access access,
				u32 offset,
				u32 size)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !buffer.valid()
					|| !valid_access(access)
					|| !valid_buffer_range(offset, size))
				{
					return false;
				}

				target.setBuffer(
					stage,
					VertexBufferAccess::native(buffer),
					static_cast<bgfx::Access::Enum>(access),
					offset,
					size);
				return true;
			}

			template <typename Target>
			bool set_buffer(
				Target target,
				bool enabled,
				u8 stage,
				const IndexBuffer& buffer,
				Access access,
				u32 offset,
				u32 size)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !buffer.valid()
					|| !valid_access(access)
					|| !valid_buffer_range(offset, size))
				{
					return false;
				}

				target.setBuffer(
					stage,
					IndexBufferAccess::native(buffer),
					static_cast<bgfx::Access::Enum>(access),
					offset,
					size);
				return true;
			}

			template <typename Target>
			bool set_buffer(
				Target target,
				bool enabled,
				u8 stage,
				const DynamicVertexBuffer& buffer,
				Access access,
				u32 offset,
				u32 size)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !buffer.valid()
					|| !valid_access(access)
					|| !valid_buffer_range(offset, size))
				{
					return false;
				}

				target.setBuffer(
					stage,
					DynamicVertexBufferAccess::native(buffer),
					static_cast<bgfx::Access::Enum>(access),
					offset,
					size);
				return true;
			}

			template <typename Target>
			bool set_buffer(
				Target target,
				bool enabled,
				u8 stage,
				const DynamicIndexBuffer& buffer,
				Access access,
				u32 offset,
				u32 size)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !buffer.valid()
					|| !valid_access(access)
					|| !valid_buffer_range(offset, size))
				{
					return false;
				}

				target.setBuffer(
					stage,
					DynamicIndexBufferAccess::native(buffer),
					static_cast<bgfx::Access::Enum>(access),
					offset,
					size);
				return true;
			}

			template <typename Target>
			bool set_buffer(
				Target target,
				bool enabled,
				u8 stage,
				const IndirectBuffer& buffer,
				Access access)
			{
				if (!enabled
					|| !valid_stage(stage)
					|| !buffer.valid()
					|| !valid_access(access))
				{
					return false;
				}

				target.setBuffer(
					stage,
					IndirectBufferAccess::native(buffer),
					static_cast<bgfx::Access::Enum>(access));
				return true;
			}

			template <typename Target>
			bool dispatch(
				Target target,
				bool enabled,
				u16 view,
				const Program& program,
				u32 numX,
				u32 numY,
				u32 numZ,
				DiscardFlags flags)
			{
				if (!enabled || !program.valid())
					return false;

				target.dispatch(
					view,
					ProgramAccess::native(program),
					numX,
					numY,
					numZ,
					static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool dispatch(
				Target target,
				bool enabled,
				u16 view,
				const Program& program,
				const IndirectBuffer& indirect,
				u32 start,
				u32 num,
				DiscardFlags flags)
			{
				const u32 count{ indirect.count() };
				if (!enabled
					|| !program.valid()
					|| num == 0
					|| start > count
					|| num > count - start)
				{
					return false;
				}

				target.dispatch(
					view,
					ProgramAccess::native(program),
					IndirectBufferAccess::native(indirect),
					start,
					num,
					static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool submit(
				Target target,
				bool enabled,
				u16 view,
				const Program& program,
				u32 depth,
				DiscardFlags flags)
			{
				if (!enabled || !program.valid())
					return false;

				target.submit(
					view,
					ProgramAccess::native(program),
					depth,
					static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool submit(
				Target target,
				bool enabled,
				u16 view,
				const Program& program,
				const OcclusionQuery& query,
				u32 depth,
				DiscardFlags flags)
			{
				if (!enabled || !program.valid() || !query.valid())
					return false;

				target.submit(
					view,
					ProgramAccess::native(program),
					OcclusionQueryAccess::native(query),
					depth,
					static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool submit(
				Target target,
				bool enabled,
				u16 view,
				const Program& program,
				const IndirectBuffer& indirect,
				u32 start,
				u32 num,
				u32 depth,
				DiscardFlags flags)
			{
				if (!enabled
					|| !program.valid()
					|| !indirect.valid()
					|| !supports_draw_indirect())
				{
					return false;
				}

				const u32 count{ indirect.count() };
				if (num == 0 || start > count || num > count - start)
					return false;

				target.submit(
					view,
					ProgramAccess::native(program),
					IndirectBufferAccess::native(indirect),
					start,
					num,
					depth,
					static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool submit(
				Target target,
				bool enabled,
				u16 view,
				const Program& program,
				const IndirectBuffer& indirect,
				u32 start,
				const IndexBuffer& numBuffer,
				u32 numIndex,
				u32 numMax,
				u32 depth,
				DiscardFlags flags)
			{
				if (!enabled
					|| !program.valid()
					|| !indirect.valid()
					|| !numBuffer.valid()
					|| !numBuffer.index_32()
					|| !numBuffer.has_flag(BufferFlags::DrawIndirect)
					|| !supports_draw_indirect()
					|| !supports_draw_indirect_count())
				{
					return false;
				}

				const u32 count{ indirect.count() };
				if (start >= count
					|| numMax == 0
					|| numIndex >= numBuffer.count())
				{
					return false;
				}

				target.submit(
					view,
					ProgramAccess::native(program),
					IndirectBufferAccess::native(indirect),
					start,
					IndexBufferAccess::native(numBuffer),
					numIndex,
					numMax,
					depth,
					static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool discard(Target target, bool enabled, DiscardFlags flags)
			{
				if (!enabled)
					return false;

				target.discard(static_cast<u8>(flags));
				return true;
			}

			template <typename Target>
			bool touch(Target target, bool enabled, u16 view)
			{
				if (!enabled)
					return false;

				target.touch(view);
				return true;
			}

			template <typename Target>
			bool blit_texture(
				Target target,
				bool enabled,
				u16 view,
				const TextureRegion& dst,
				const TextureRegion& src)
			{
				if (!enabled || !TextureAccess::blit_valid(dst, src))
					return false;

				const bgfx::Caps* caps{ bgfx::getCaps() };
				if (!caps || view >= caps->limits.maxViews)
					return false;

				target.blit(
					view,
					TextureAccess::region(dst),
					TextureAccess::region(src));
				return true;
			}
		} // namespace draw
	} // namespace detail
}

#endif
