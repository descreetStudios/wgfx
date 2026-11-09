#pragma once

#include <wcore/types.hpp>

#include <memory>

namespace wgfx
{
	enum class Renderer : u8
	{
		Auto,
		Noop,
		Direct3D11,
		Direct3D12,
		Metal,
		OpenGL,
		OpenGLES,
		Vulkan,
		WebGPU,
	};

	enum class NativeWindowType : u8
	{
		Default,
		Wayland,
	};

	struct NativeWindow
	{
		void* handle{ nullptr };
		void* display{ nullptr };
		NativeWindowType type{ NativeWindowType::Default };
	};

	struct DeviceDesc
	{
		NativeWindow window{};
		u32 width{ 1280 };
		u32 height{ 720 };
		Renderer renderer{ Renderer::Auto };
		bool vsync{ true };
		bool debug{ false };
		bool profile{ false };
	};

	struct DeviceCaps
	{
		Renderer renderer{ Renderer::Auto };
		bool homogeneousDepth{ false };
		bool originBottomLeft{ false };
		u16 vendorId{ 0 };
		u16 deviceId{ 0 };
		u32 maxDrawCalls{ 0 };
		u32 maxTextureSize{ 0 };
		u32 maxTextureLayers{ 0 };
		u32 maxViews{ 0 };
		u32 maxFrameBuffers{ 0 };
		u32 maxTransientVbSize{ 0 };
		u32 maxTransientIbSize{ 0 };
		u32 maxInstanceData{ 0 };
		bool swapChain{ false };
		bool drawIndirect{ false };
		bool drawIndirectCount{ false };
		bool variableRateShading{ false };
	};

	class Device
	{
	public:
		static std::unique_ptr<Device> create(const DeviceDesc& desc);

		~Device();

		Device(const Device&) = delete;
		Device& operator=(const Device&) = delete;
		Device(Device&&) = delete;
		Device& operator=(Device&&) = delete;

		bool valid() const;

		void shutdown();

		const DeviceDesc& desc() const;
		const DeviceCaps& caps() const;

		Renderer renderer() const;
		const char* renderer_name() const;

		u32 width() const;
		u32 height() const;
		bool vsync() const;

		void resize(u32 width, u32 height);

		bool reset();

		void set_vsync(bool enabled);

		u32 frame();

	private:
		explicit Device(const DeviceDesc& desc);

		bool initialize();
		void destroy();

		DeviceDesc mDesc;
		DeviceCaps mCaps;
		const char* mRendererName{ "none" };
		bool mValid{ false };
		u32 mFrame{ 0 };
	};
}
