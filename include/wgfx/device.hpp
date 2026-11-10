#pragma once

#include <wgfx/diagnostics.hpp>

#include <wcore/types.hpp>

#include <memory>

namespace wgfx
{
	class Callback;
	class FrameBuffer;

	namespace detail
	{
		class CallbackAdapter;
	}

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

	enum class RenderFrame : u8
	{
		NoContext,
		Render,
		Timeout,
		Exiting,

		Count,
	};

	// Bit flags matching bgfx's BGFX_FRAME_* values.
	enum class FrameFlags : u8
	{
		None = 0x00,
		DebugCapture = 0x01,
		Discard = 0x02,
		Flush = 0x04,
	};

	constexpr FrameFlags operator|(FrameFlags lhs, FrameFlags rhs) noexcept
	{
		return static_cast<FrameFlags>(
			static_cast<u8>(lhs) | static_cast<u8>(rhs));
	}

	constexpr FrameFlags operator&(FrameFlags lhs, FrameFlags rhs) noexcept
	{
		return static_cast<FrameFlags>(
			static_cast<u8>(lhs) & static_cast<u8>(rhs));
	}

	constexpr FrameFlags& operator|=(FrameFlags& lhs, FrameFlags rhs) noexcept
	{
		lhs = lhs | rhs;
		return lhs;
	}

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
		u16 maxEncoders{ 0 };

		Callback* callback{ nullptr };
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
		u32 maxEncoders{ 0 };
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

		bool reset() const;

		void set_vsync(bool enabled);

		u32 frame();
		u32 frame(FrameFlags flags);

		RenderFrame render_frame(i32 timeout = -1) const;

		bool request_screen_shot(
			const FrameBuffer& framebuffer,
			const char* filePath) const;

		bool set_debug(DebugFlags flags,
			const FrameBuffer* framebuffer = nullptr,
			u8 scale = 0) const;
		Stats stats() const;

		bool debug_text_clear(u8 attr = 0, bool small = false);
		bool debug_text_printf(
			u16 x,
			u16 y,
			u8 attr,
			const char* format,
			...);
		bool debug_text_image(
			u16 x,
			u16 y,
			u16 width,
			u16 height,
			const void* data,
			u16 pitch);
		bool set_marker(const char* name, i32 length = -1);

	private:
		explicit Device(const DeviceDesc& desc);

		bool initialize();
		void destroy();

		DeviceDesc mDesc;
		DeviceCaps mCaps;
		const char* mRendererName{ "none" };
		bool mValid{ false };
		u32 mFrame{ 0 };
		std::unique_ptr<detail::CallbackAdapter> mCallback;
	};
}
