#include <wgfx/device.hpp>

#include "callback_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include "framebuffer_internal.hpp"

#include <bgfx/bgfx.h>

#include <cstdarg>
#include <cstring>
#include <limits>
#endif

namespace wgfx
{
	namespace
	{
#if defined(WGFX_BACKEND_BGFX)
		bool gActive{ false };
		u32 gGeneration{ 0 };

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

			return bgfx::RendererType::Count;
		}

		Renderer from_bgfx(bgfx::RendererType::Enum renderer)
		{
			switch (renderer)
			{
			case bgfx::RendererType::Noop: return Renderer::Noop;
			case bgfx::RendererType::Direct3D11: return Renderer::Direct3D11;
			case bgfx::RendererType::Direct3D12: return Renderer::Direct3D12;
			case bgfx::RendererType::Metal: return Renderer::Metal;
			case bgfx::RendererType::OpenGL: return Renderer::OpenGL;
			case bgfx::RendererType::OpenGLES: return Renderer::OpenGLES;
			case bgfx::RendererType::Vulkan: return Renderer::Vulkan;
			case bgfx::RendererType::WebGPU: return Renderer::WebGPU;
			default: break;
			}

			return Renderer::Auto;
		}

		bgfx::NativeWindowHandleType::Enum to_bgfx(NativeWindowType type)
		{
			switch (type)
			{
			case NativeWindowType::Wayland: return bgfx::NativeWindowHandleType::Wayland;
			case NativeWindowType::Default: break;
			}

			return bgfx::NativeWindowHandleType::Default;
		}

		u32 reset_flags(const DeviceDesc& desc)
		{
			return desc.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
		}

		bgfx::SwapChain make_swap_chain(const DeviceDesc& desc)
		{
			bgfx::SwapChain swapChain;
			swapChain.nwh = desc.window.handle;
			swapChain.ndt = desc.window.display;
			swapChain.width = desc.width;
			swapChain.height = desc.height;
			return swapChain;
		}

#define WGFX_DEBUG_MIRROR(name, bgfxName) \
		static_assert(static_cast<u32>(DebugFlags::name) == (bgfxName), \
			"DebugFlags::" #name " must mirror " #bgfxName)

		WGFX_DEBUG_MIRROR(None, BGFX_DEBUG_NONE);
		WGFX_DEBUG_MIRROR(Wireframe, BGFX_DEBUG_WIREFRAME);
		WGFX_DEBUG_MIRROR(InfinitelyFastHardware, BGFX_DEBUG_IFH);
		WGFX_DEBUG_MIRROR(Stats, BGFX_DEBUG_STATS);
		WGFX_DEBUG_MIRROR(Text, BGFX_DEBUG_TEXT);
		WGFX_DEBUG_MIRROR(Profiler, BGFX_DEBUG_PROFILER);

#undef WGFX_DEBUG_MIRROR

#define WGFX_TOPOLOGY_MIRROR(name, bgfxName) \
		static_assert(static_cast<u8>(PrimitiveTopology::name) == (bgfxName), \
			"PrimitiveTopology::" #name " must mirror " #bgfxName)

		WGFX_TOPOLOGY_MIRROR(TriangleList, bgfx::Topology::TriList);
		WGFX_TOPOLOGY_MIRROR(TriangleStrip, bgfx::Topology::TriStrip);
		WGFX_TOPOLOGY_MIRROR(LineList, bgfx::Topology::LineList);
		WGFX_TOPOLOGY_MIRROR(LineStrip, bgfx::Topology::LineStrip);
		WGFX_TOPOLOGY_MIRROR(PointList, bgfx::Topology::PointList);
		WGFX_TOPOLOGY_MIRROR(Count, bgfx::Topology::Count);

#undef WGFX_TOPOLOGY_MIRROR

#define WGFX_RENDER_FRAME_MIRROR(name) \
		static_assert(static_cast<u8>(RenderFrame::name) \
			== bgfx::RenderFrame::name, \
			"RenderFrame::" #name " must mirror bgfx::RenderFrame::" #name)

		WGFX_RENDER_FRAME_MIRROR(NoContext);
		WGFX_RENDER_FRAME_MIRROR(Render);
		WGFX_RENDER_FRAME_MIRROR(Timeout);
		WGFX_RENDER_FRAME_MIRROR(Exiting);
		WGFX_RENDER_FRAME_MIRROR(Count);

#undef WGFX_RENDER_FRAME_MIRROR

#define WGFX_FRAME_FLAG_MIRROR(name, bgfxName) \
		static_assert(static_cast<u8>(FrameFlags::name) == (bgfxName), \
			"FrameFlags::" #name " must mirror " #bgfxName)

		WGFX_FRAME_FLAG_MIRROR(None, BGFX_FRAME_NONE);
		WGFX_FRAME_FLAG_MIRROR(DebugCapture, BGFX_FRAME_DEBUG_CAPTURE);
		WGFX_FRAME_FLAG_MIRROR(Discard, BGFX_FRAME_DISCARD);
		WGFX_FRAME_FLAG_MIRROR(Flush, BGFX_FRAME_FLUSH);

#undef WGFX_FRAME_FLAG_MIRROR
#endif
	} // anonymous namespace

	Device::Device(const DeviceDesc& desc)
		: mDesc{ desc }
	{
	}

	Device::~Device()
	{
		shutdown();
	}

	std::unique_ptr<Device> Device::create(const DeviceDesc& desc)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (gActive)
			return nullptr;

		std::unique_ptr<Device> device{ new Device(desc) };
		if (!device->initialize())
			return nullptr;

		return device;
#else
		(void)desc;
		return nullptr;
#endif
	}

	bool Device::initialize()
	{
#if defined(WGFX_BACKEND_BGFX)
		bgfx::Init init;
		init.type = to_bgfx(mDesc.renderer);
		init.debug = mDesc.debug;
		init.profile = mDesc.profile;
		init.platformData.type = to_bgfx(mDesc.window.type);
		init.swapChain.nwh = mDesc.window.handle;
		init.swapChain.ndt = mDesc.window.display;
		init.swapChain.width = mDesc.width;
		init.swapChain.height = mDesc.height;
		init.reset = reset_flags(mDesc);
		if (mDesc.maxEncoders > 0)
			init.limits.maxEncoders = mDesc.maxEncoders;

		mCallback = std::make_unique<detail::CallbackAdapter>(mDesc.callback);
		init.callback = mCallback.get();

		if (!bgfx::init(init))
			return false;

		gActive = true;
		++gGeneration;
		mValid = true;

		if (const bgfx::Caps * caps{ bgfx::getCaps() })
		{
			mCaps.renderer = from_bgfx(caps->rendererType);
			mCaps.homogeneousDepth = caps->homogeneousDepth;
			mCaps.originBottomLeft = caps->originBottomLeft;
			mCaps.vendorId = caps->vendorId;
			mCaps.deviceId = caps->deviceId;
			mCaps.maxDrawCalls = caps->limits.maxDrawCalls;
			mCaps.maxTextureSize = caps->limits.maxTextureSize;
			mCaps.maxTextureLayers = caps->limits.maxTextureLayers;
			mCaps.maxViews = caps->limits.maxViews;
			mCaps.maxFrameBuffers = caps->limits.maxFrameBuffers;
			mCaps.maxTransientVbSize = caps->limits.maxTransientVbSize;
			mCaps.maxTransientIbSize = caps->limits.maxTransientIbSize;
			mCaps.maxInstanceData = caps->limits.maxInstanceData;
			mCaps.maxEncoders = caps->limits.maxEncoders;
			mCaps.swapChain = 0 != (caps->supported & BGFX_CAPS_SWAP_CHAIN);
			mCaps.drawIndirect = 0 != (caps->supported & BGFX_CAPS_DRAW_INDIRECT);
			mCaps.drawIndirectCount =
				0 != (caps->supported & BGFX_CAPS_DRAW_INDIRECT_COUNT);
			mCaps.variableRateShading =
				0 != (caps->supported & BGFX_CAPS_VARIABLE_RATE_SHADING);
		}

		mRendererName = bgfx::getRendererName(bgfx::getRendererType());
		return true;
#else
		return false;
#endif
	}

	void Device::destroy()
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid)
			return;

		bgfx::shutdown();
		mCallback.reset();
		mValid = false;
		gActive = false;
		++gGeneration;
		mRendererName = "none";
#else
		mValid = false;
#endif
	}

	void Device::shutdown()
	{
		destroy();
	}

	bool Device::valid() const
	{
		return mValid;
	}

	const DeviceDesc& Device::desc() const
	{
		return mDesc;
	}

	const DeviceCaps& Device::caps() const
	{
		return mCaps;
	}

	Renderer Device::renderer() const
	{
		return mCaps.renderer;
	}

	const char* Device::renderer_name() const
	{
		return mRendererName;
	}

	u32 Device::width() const
	{
		return mDesc.width;
	}

	u32 Device::height() const
	{
		return mDesc.height;
	}

	bool Device::vsync() const
	{
		return mDesc.vsync;
	}

	void Device::resize(u32 width, u32 height)
	{
		if (width == 0 || height == 0)
			return;

		mDesc.width = width;
		mDesc.height = height;
		reset();
	}

	bool Device::reset() const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid || mDesc.width == 0 || mDesc.height == 0)
			return false;

		bgfx::SwapChain swapChain{ make_swap_chain(mDesc) };
		bgfx::reset(reset_flags(mDesc), mDesc.window.handle ? &swapChain : nullptr);
		return true;
#else
		return false;
#endif
	}

	void Device::set_vsync(bool enabled)
	{
		if (mDesc.vsync == enabled)
			return;

		mDesc.vsync = enabled;
		reset();
	}

	u32 Device::frame()
	{
		return frame(FrameFlags::None);
	}

	u32 Device::frame(FrameFlags flags)
	{
#if defined(WGFX_BACKEND_BGFX)
		constexpr u8 FrameFlagsMask{ 0x07 };

		if (!mValid || (static_cast<u8>(flags) & ~FrameFlagsMask) != 0)
			return 0;

		mFrame = bgfx::frame(static_cast<u8>(flags));
		return mFrame;
#else
		(void)flags;
		return 0;
#endif
	}

	RenderFrame Device::render_frame(i32 timeout) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid)
			return RenderFrame::NoContext;

		return static_cast<RenderFrame>(bgfx::renderFrame(timeout));
#else
		(void)timeout;
		return RenderFrame::NoContext;
#endif
	}

	bool Device::request_screen_shot(
		const FrameBuffer& framebuffer,
		const char* filePath) const
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid
			|| !framebuffer.valid()
			|| !framebuffer.is_swap_chain()
			|| !filePath
			|| filePath[0] == '\0')
		{
			return false;
		}

		bgfx::requestScreenShot(
			detail::FrameBufferAccess::native(framebuffer), filePath);
		return true;
#else
		(void)framebuffer;
		(void)filePath;
		return false;
#endif
	}

	bool Device::set_debug(
		DebugFlags flags,
		const FrameBuffer* framebuffer,
		u8 scale) const
	{
#if defined(WGFX_BACKEND_BGFX)
		constexpr u32 DebugFlagsMask{ 0x1f };

		if (!mValid
			|| (framebuffer && !framebuffer->valid())
			|| (static_cast<u32>(flags) & ~DebugFlagsMask) != 0)
		{
			return false;
		}

		const bgfx::FrameBufferHandle handle{ framebuffer
			? detail::FrameBufferAccess::native(*framebuffer)
			: bgfx::FrameBufferHandle{ bgfx::kInvalidHandle } };
		bgfx::setDebug(static_cast<u32>(flags), handle, scale);
		return true;
#else
		(void)flags;
		(void)framebuffer;
		(void)scale;
		return false;
#endif
	}

	Stats Device::stats() const
	{
		Stats result;

#if defined(WGFX_BACKEND_BGFX)
		if (!mValid)
			return result;

		const bgfx::Stats* stats{ bgfx::getStats() };
		if (!stats)
			return result;

		result.cpuTimeFrame = stats->cpuTimeFrame;
		result.cpuTimeBegin = stats->cpuTimeBegin;
		result.cpuTimeEnd = stats->cpuTimeEnd;
		result.cpuTimerFreq = stats->cpuTimerFreq;
		result.gpuTimeBegin = stats->gpuTimeBegin;
		result.gpuTimeEnd = stats->gpuTimeEnd;
		result.gpuTimerFreq = stats->gpuTimerFreq;
		result.waitRender = stats->waitRender;
		result.waitSubmit = stats->waitSubmit;
		result.latencySleep = stats->latencySleep;
		result.latencyTotal = stats->latencyTotal;
		result.latencySimulation = stats->latencySimulation;
		result.latencyRenderSubmit = stats->latencyRenderSubmit;
		result.latencyPresent = stats->latencyPresent;
		result.latencyQueue = stats->latencyQueue;
		result.latencyGpu = stats->latencyGpu;

		result.numDraw = stats->numDraw;
		result.numCompute = stats->numCompute;
		result.numBlit = stats->numBlit;
		result.numBlitRepack = stats->numBlitRepack;
		result.numDrawCallsPeak = stats->numDrawCallsPeak;
		result.maxGpuLatency = stats->maxGpuLatency;
		result.gpuFrameNum = stats->gpuFrameNum;

		result.numDynamicIndexBuffers = stats->numDynamicIndexBuffers;
		result.numDynamicVertexBuffers = stats->numDynamicVertexBuffers;
		result.numFrameBuffers = stats->numFrameBuffers;
		result.numIndexBuffers = stats->numIndexBuffers;
		result.numOcclusionQueries = stats->numOcclusionQueries;
		result.numPrograms = stats->numPrograms;
		result.numShaders = stats->numShaders;
		result.numTextures = stats->numTextures;
		result.numUniforms = stats->numUniforms;
		result.numVertexBuffers = stats->numVertexBuffers;
		result.numVertexLayouts = stats->numVertexLayouts;

		result.textureMemoryUsed = stats->textureMemoryUsed;
		result.rtMemoryUsed = stats->rtMemoryUsed;
		result.transientVbUsed = stats->transientVbUsed;
		result.transientIbUsed = stats->transientIbUsed;

		constexpr u32 TopologyCount{
			static_cast<u32>(PrimitiveTopology::Count) };
		for (u32 index = 0; index < TopologyCount; ++index)
			result.numPrims[index] = stats->numPrims[index];

		result.gpuMemoryMax = stats->gpuMemoryMax;
		result.gpuMemoryUsed = stats->gpuMemoryUsed;

		result.width = stats->width;
		result.height = stats->height;
		result.textWidth = stats->textWidth;
		result.textHeight = stats->textHeight;

		if (stats->viewStats)
		{
			result.views.resize(stats->numViews);
			for (u32 index = 0; index < stats->numViews; ++index)
			{
				const bgfx::ViewStats& source{ stats->viewStats[index] };
				ViewStats& target{ result.views[index] };
				std::memcpy(target.name, source.name, sizeof(target.name));
				target.view = source.view;
				target.cpuTimeBegin = source.cpuTimeBegin;
				target.cpuTimeEnd = source.cpuTimeEnd;
				target.gpuTimeBegin = source.gpuTimeBegin;
				target.gpuTimeEnd = source.gpuTimeEnd;
				target.gpuFrameNum = source.gpuFrameNum;
			}
		}

		if (stats->encoderStats)
		{
			result.encoders.resize(stats->numEncoders);
			for (u32 index = 0; index < stats->numEncoders; ++index)
			{
				const bgfx::EncoderStats& source{ stats->encoderStats[index] };
				EncoderStats& target{ result.encoders[index] };
				target.cpuTimeBegin = source.cpuTimeBegin;
				target.cpuTimeEnd = source.cpuTimeEnd;
			}
		}
#endif

		return result;
	}

	bool Device::debug_text_clear(u8 attr, bool small)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid)
			return false;

		bgfx::dbgTextClear(attr, small);
		return true;
#else
		(void)attr;
		(void)small;
		return false;
#endif
	}

	bool Device::debug_text_printf(
		u16 x, u16 y, u8 attr,
		const char* format,
		...)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid || !format)
			return false;

		va_list args;
		va_start(args, format);
		bgfx::dbgTextPrintfVargs(x, y, attr, format, args);
		va_end(args);
		return true;
#else
		(void)x;
		(void)y;
		(void)attr;
		(void)format;
		return false;
#endif
	}

	bool Device::debug_text_image(
		u16 x, u16 y,
		u16 width, u16 height,
		const void* data,
		u16 pitch)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid || !data)
			return false;

		bgfx::dbgTextImage(x, y, width, height, data, pitch);
		return true;
#else
		(void)x;
		(void)y;
		(void)width;
		(void)height;
		(void)data;
		(void)pitch;
		return false;
#endif
	}

	bool Device::set_marker(const char* name, i32 length)
	{
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid || !name)
			return false;

		const i32 markerLength{ length < 0
			? std::numeric_limits<i32>::max()
			: length };
		bgfx::setMarker(name, markerLength);
		return true;
#else
		(void)name;
		(void)length;
		return false;
#endif
	}

	namespace detail
	{
#if defined(WGFX_BACKEND_BGFX)
		bool device_active()
		{
			return gActive;
		}

		u32 device_generation()
		{
			return gGeneration;
		}
#else
		bool device_active()
		{
			return false;
		}

		u32 device_generation()
		{
			return 0;
		}
#endif
	} // namespace detail
}