#pragma once

#include <wcore/types.hpp>

#include <vector>

namespace wgfx
{
	// Bit flags matching bgfx's BGFX_DEBUG_* values.
	enum class DebugFlags : u32
	{
		None = 0x00000000,
		Wireframe = 0x00000001,
		InfinitelyFastHardware = 0x00000002,
		Stats = 0x00000004,
		Text = 0x00000008,
		Profiler = 0x00000010,
	};

	constexpr DebugFlags operator|(DebugFlags lhs, DebugFlags rhs) noexcept
	{
		return static_cast<DebugFlags>(
			static_cast<u32>(lhs) | static_cast<u32>(rhs));
	}

	constexpr DebugFlags operator&(DebugFlags lhs, DebugFlags rhs) noexcept
	{
		return static_cast<DebugFlags>(
			static_cast<u32>(lhs) & static_cast<u32>(rhs));
	}

	constexpr DebugFlags& operator|=(DebugFlags& lhs, DebugFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

	enum class PrimitiveTopology : u8
	{
		TriangleList,
		TriangleStrip,
		LineList,
		LineStrip,
		PointList,

		Count,
	};

	struct ViewStats
	{
		static constexpr u32 MaxNameLength{ 255 };

		char name[MaxNameLength + 1]{};
		u16 view{ 0 };
		i64 cpuTimeBegin{ 0 };
		i64 cpuTimeEnd{ 0 };
		i64 gpuTimeBegin{ 0 };
		i64 gpuTimeEnd{ 0 };
		u32 gpuFrameNum{ 0 };
	};

	struct EncoderStats
	{
		i64 cpuTimeBegin{ 0 };
		i64 cpuTimeEnd{ 0 };
	};

	struct Stats
	{
		i64 cpuTimeFrame{ 0 };
		i64 cpuTimeBegin{ 0 };
		i64 cpuTimeEnd{ 0 };
		i64 cpuTimerFreq{ 0 };
		i64 gpuTimeBegin{ 0 };
		i64 gpuTimeEnd{ 0 };
		i64 gpuTimerFreq{ 0 };
		i64 waitRender{ 0 };
		i64 waitSubmit{ 0 };
		i64 latencySleep{ 0 };
		i64 latencyTotal{ 0 };
		i64 latencySimulation{ 0 };
		i64 latencyRenderSubmit{ 0 };
		i64 latencyPresent{ 0 };
		i64 latencyQueue{ 0 };
		i64 latencyGpu{ 0 };

		u32 numDraw{ 0 };
		u32 numCompute{ 0 };
		u32 numBlit{ 0 };
		u32 numBlitRepack{ 0 };
		u32 numDrawCallsPeak{ 0 };
		u32 maxGpuLatency{ 0 };
		u32 gpuFrameNum{ 0 };

		u16 numDynamicIndexBuffers{ 0 };
		u16 numDynamicVertexBuffers{ 0 };
		u16 numFrameBuffers{ 0 };
		u16 numIndexBuffers{ 0 };
		u16 numOcclusionQueries{ 0 };
		u16 numPrograms{ 0 };
		u16 numShaders{ 0 };
		u16 numTextures{ 0 };
		u16 numUniforms{ 0 };
		u16 numVertexBuffers{ 0 };
		u16 numVertexLayouts{ 0 };

		i64 textureMemoryUsed{ 0 };
		i64 rtMemoryUsed{ 0 };
		i32 transientVbUsed{ 0 };
		i32 transientIbUsed{ 0 };

		u32 numPrims[static_cast<u32>(PrimitiveTopology::Count)]{};

		i64 gpuMemoryMax{ 0 };
		i64 gpuMemoryUsed{ 0 };

		u16 width{ 0 };
		u16 height{ 0 };
		u16 textWidth{ 0 };
		u16 textHeight{ 0 };

		std::vector<ViewStats> views;
		std::vector<EncoderStats> encoders;
	};
}