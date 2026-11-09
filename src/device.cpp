#include <wgfx/device.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>
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

		if (!bgfx::init(init))
			return false;

		gActive = true;
		++gGeneration;
		mValid = true;

		if (const bgfx::Caps* caps{ bgfx::getCaps() })
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
			mCaps.swapChain = 0 != (caps->supported & BGFX_CAPS_SWAP_CHAIN);
			mCaps.drawIndirect = 0 != (caps->supported & BGFX_CAPS_DRAW_INDIRECT);
			mCaps.drawIndirectCount =
				0 != (caps->supported & BGFX_CAPS_DRAW_INDIRECT_COUNT);
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

	bool Device::reset()
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
#if defined(WGFX_BACKEND_BGFX)
		if (!mValid)
			return 0;

		mFrame = bgfx::frame();
		return mFrame;
#else
		return 0;
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
