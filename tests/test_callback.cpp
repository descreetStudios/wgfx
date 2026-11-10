#include <wcore/test.hpp>
#include <wgfx/wgfx.hpp>

#include <string>

#if defined(WGFX_BACKEND_BGFX)
#include "callback_internal.hpp"

#include <cstdarg>
#endif

using namespace wcore;
using namespace test;

namespace
{
	wgfx::DeviceDesc headless_desc()
	{
		wgfx::DeviceDesc desc;
		desc.renderer = wgfx::Renderer::Noop;
		desc.width = 320;
		desc.height = 180;
		desc.vsync = false;
		return desc;
	}

	wgfx::SwapChainDesc swap_chain_desc()
	{
		wgfx::SwapChainDesc desc;
		desc.width = 64;
		desc.height = 48;
		desc.colorFormat = wgfx::TextureFormat::RGBA8;
		return desc;
	}

	wgfx::FrameBufferDesc color_framebuffer_desc()
	{
		wgfx::FrameBufferDesc desc;
		desc.width = 16;
		desc.height = 16;
		desc.colorFormat = wgfx::TextureFormat::RGBA8;
		return desc;
	}
} // anonymous namespace

TEST(callback_frame_flags_compose)
{
	CHECK_EQ(static_cast<u8>(wgfx::FrameFlags::None), 0u);
	CHECK_EQ(static_cast<u8>(wgfx::FrameFlags::DebugCapture), 0x01u);
	CHECK_EQ(static_cast<u8>(wgfx::FrameFlags::Discard), 0x02u);
	CHECK_EQ(static_cast<u8>(wgfx::FrameFlags::Flush), 0x04u);

	const wgfx::FrameFlags flags{
		wgfx::FrameFlags::DebugCapture | wgfx::FrameFlags::Flush };
	CHECK((flags & wgfx::FrameFlags::DebugCapture)
		== wgfx::FrameFlags::DebugCapture);
	CHECK((flags & wgfx::FrameFlags::Flush) == wgfx::FrameFlags::Flush);
	CHECK((flags & wgfx::FrameFlags::Discard) == wgfx::FrameFlags::None);

	wgfx::FrameFlags mutableFlags{ wgfx::FrameFlags::None };
	mutableFlags |= wgfx::FrameFlags::Discard;
	CHECK((mutableFlags & wgfx::FrameFlags::Discard)
		== wgfx::FrameFlags::Discard);
}

#if defined(WGFX_BACKEND_BGFX)

namespace
{
	struct RecordingCallback final : wgfx::Callback
	{
		void fatal(
			const char* filePath,
			u16 line,
			wgfx::FatalError code,
			const char* message) override
		{
			++fatalCount;
			fatalFile = filePath ? filePath : "";
			fatalLine = line;
			fatalCode = code;
			fatalMessage = message ? message : "";
		}

		void trace(
			const char* filePath,
			u16 line,
			const char* message) override
		{
			++traceCount;
			traceFile = filePath ? filePath : "";
			traceLine = line;
			traceMessage = message ? message : "";
		}

		void profiler_begin(
			const char* name,
			u32 abgr,
			const char* filePath,
			u16 line) override
		{
			++profilerBeginCount;
			profilerName = name ? name : "";
			profilerAbgr = abgr;
			profilerFile = filePath ? filePath : "";
			profilerLine = line;
		}

		void profiler_begin_literal(
			const char* name,
			u32 abgr,
			const char* filePath,
			u16 line) override
		{
			++profilerLiteralCount;
			profilerLiteralName = name ? name : "";
			profilerLiteralAbgr = abgr;
			profilerLiteralFile = filePath ? filePath : "";
			profilerLiteralLine = line;
		}

		void profiler_end() override
		{
			++profilerEndCount;
		}

		void screen_shot(
			const char* filePath,
			u32 width,
			u32 height,
			u32 pitch,
			wgfx::TextureFormat format,
			const void* data,
			u32 size,
			bool yflip) override
		{
			++screenShotCount;
			screenShotPath = filePath ? filePath : "";
			screenShotWidth = width;
			screenShotHeight = height;
			screenShotPitch = pitch;
			screenShotFormat = format;
			screenShotData = data;
			screenShotSize = size;
			screenShotYFlip = yflip;
		}

		void capture_begin(
			u32 width,
			u32 height,
			u32 pitch,
			wgfx::TextureFormat format,
			bool yflip) override
		{
			++captureBeginCount;
			captureWidth = width;
			captureHeight = height;
			capturePitch = pitch;
			captureFormat = format;
			captureYFlip = yflip;
		}

		void capture_end() override
		{
			++captureEndCount;
		}

		void capture_frame(const void* data, u32 size) override
		{
			++captureFrameCount;
			captureData = data;
			captureSize = size;
		}

		u32 fatalCount{ 0 };
		std::string fatalFile;
		u16 fatalLine{ 0 };
		wgfx::FatalError fatalCode{ wgfx::FatalError::Count };
		std::string fatalMessage;

		u32 traceCount{ 0 };
		std::string traceFile;
		u16 traceLine{ 0 };
		std::string traceMessage;

		u32 profilerBeginCount{ 0 };
		std::string profilerName;
		u32 profilerAbgr{ 0 };
		std::string profilerFile;
		u16 profilerLine{ 0 };

		u32 profilerLiteralCount{ 0 };
		std::string profilerLiteralName;
		u32 profilerLiteralAbgr{ 0 };
		std::string profilerLiteralFile;
		u16 profilerLiteralLine{ 0 };

		u32 profilerEndCount{ 0 };

		u32 screenShotCount{ 0 };
		std::string screenShotPath;
		u32 screenShotWidth{ 0 };
		u32 screenShotHeight{ 0 };
		u32 screenShotPitch{ 0 };
		wgfx::TextureFormat screenShotFormat{ wgfx::TextureFormat::Count };
		const void* screenShotData{ nullptr };
		u32 screenShotSize{ 0 };
		bool screenShotYFlip{ false };

		u32 captureBeginCount{ 0 };
		u32 captureWidth{ 0 };
		u32 captureHeight{ 0 };
		u32 capturePitch{ 0 };
		wgfx::TextureFormat captureFormat{ wgfx::TextureFormat::Count };
		bool captureYFlip{ false };
		u32 captureEndCount{ 0 };
		u32 captureFrameCount{ 0 };
		const void* captureData{ nullptr };
		u32 captureSize{ 0 };
	};

	void invoke_trace(
		wgfx::detail::CallbackAdapter& adapter,
		const char* format,
		...)
	{
		va_list args;
		va_start(args, format);
		adapter.traceVargs("trace.cpp", 42, format, args);
		va_end(args);
	}
} // anonymous namespace

TEST(callback_adapter_forwards_events)
{
	RecordingCallback callback;
	wgfx::detail::CallbackAdapter adapter{ &callback };

	const u8 pixels[]{ 1, 2, 3, 4 };

	adapter.fatal("fatal.cpp", 7, bgfx::Fatal::DeviceLost, "device lost");
	CHECK_EQ(callback.fatalCount, 1u);
	CHECK_EQ(callback.fatalFile, std::string("fatal.cpp"));
	CHECK_EQ(callback.fatalLine, 7u);
	CHECK(callback.fatalCode == wgfx::FatalError::DeviceLost);
	CHECK_EQ(callback.fatalMessage, std::string("device lost"));

	invoke_trace(adapter, "frame %u/%s", 12u, "ok");
	CHECK_EQ(callback.traceCount, 1u);
	CHECK_EQ(callback.traceFile, std::string("trace.cpp"));
	CHECK_EQ(callback.traceLine, 42u);
	CHECK_EQ(callback.traceMessage, std::string("frame 12/ok"));

	adapter.profilerBegin("dynamic", 0x11223344u, "profiler.cpp", 3);
	adapter.profilerBeginLiteral("literal", 0x55667788u, "profiler.cpp", 4);
	adapter.profilerEnd();
	CHECK_EQ(callback.profilerBeginCount, 1u);
	CHECK_EQ(callback.profilerName, std::string("dynamic"));
	CHECK_EQ(callback.profilerAbgr, 0x11223344u);
	CHECK_EQ(callback.profilerFile, std::string("profiler.cpp"));
	CHECK_EQ(callback.profilerLine, 3u);
	CHECK_EQ(callback.profilerLiteralCount, 1u);
	CHECK_EQ(callback.profilerLiteralName, std::string("literal"));
	CHECK_EQ(callback.profilerLiteralAbgr, 0x55667788u);
	CHECK_EQ(callback.profilerLiteralFile, std::string("profiler.cpp"));
	CHECK_EQ(callback.profilerLiteralLine, 4u);
	CHECK_EQ(callback.profilerEndCount, 1u);

	adapter.screenShot(
		"shot", 1, 1, 4,
		bgfx::TextureFormat::RGBA8, pixels, sizeof(pixels), true);
	CHECK_EQ(callback.screenShotCount, 1u);
	CHECK_EQ(callback.screenShotPath, std::string("shot"));
	CHECK_EQ(callback.screenShotWidth, 1u);
	CHECK_EQ(callback.screenShotHeight, 1u);
	CHECK_EQ(callback.screenShotPitch, 4u);
	CHECK(callback.screenShotFormat == wgfx::TextureFormat::RGBA8);
	CHECK(callback.screenShotData == pixels);
	CHECK_EQ(callback.screenShotSize, 4u);
	CHECK(callback.screenShotYFlip);

	adapter.captureBegin(
		2, 3, 8, bgfx::TextureFormat::BGRA8, false);
	adapter.captureFrame(pixels, sizeof(pixels));
	adapter.captureEnd();
	CHECK_EQ(callback.captureBeginCount, 1u);
	CHECK_EQ(callback.captureWidth, 2u);
	CHECK_EQ(callback.captureHeight, 3u);
	CHECK_EQ(callback.capturePitch, 8u);
	CHECK(callback.captureFormat == wgfx::TextureFormat::BGRA8);
	CHECK(!callback.captureYFlip);
	CHECK_EQ(callback.captureFrameCount, 1u);
	CHECK(callback.captureData == pixels);
	CHECK_EQ(callback.captureSize, 4u);
	CHECK_EQ(callback.captureEndCount, 1u);

	CHECK_EQ(adapter.cacheReadSize(1), 0u);
	CHECK(!adapter.cacheRead(1, nullptr, 0));
	adapter.cacheWrite(1, nullptr, 0);
}

TEST(callback_adapter_default_path)
{
	wgfx::detail::CallbackAdapter adapter{ nullptr };

	invoke_trace(adapter, "ignored %u/%s", 7u, "default");
	adapter.profilerBegin("ignored", 0, "ignored.cpp", 1);
	adapter.profilerBeginLiteral("ignored", 0, "ignored.cpp", 2);
	adapter.profilerEnd();
	adapter.screenShot(
		"ignored", 0, 0, 0, bgfx::TextureFormat::RGBA8,
		nullptr, 0, false);
	adapter.captureBegin(0, 0, 0, bgfx::TextureFormat::RGBA8, false);
	adapter.captureFrame(nullptr, 0);
	adapter.captureEnd();
}

TEST(callback_device_accepts_callback)
{
	RecordingCallback callback;

	wgfx::DeviceDesc desc{ headless_desc() };
	desc.callback = &callback;

	auto device = wgfx::Device::create(desc);
	REQUIRE(device != nullptr);

	CHECK(device->desc().callback == &callback);
	CHECK(device->frame() > 0);

	device->shutdown();
}

TEST(callback_frame_advances_with_flags)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);

	const u32 first{ device->frame() };
	CHECK(first > 0);

	const u32 second{ device->frame(wgfx::FrameFlags::DebugCapture) };
	CHECK(second > first);

	const u32 third{ device->frame(
		wgfx::FrameFlags::DebugCapture | wgfx::FrameFlags::Flush) };
	CHECK(third > second);

	CHECK_EQ(device->frame(
		static_cast<wgfx::FrameFlags>(0x08)), 0u);

	device->shutdown();
	CHECK_EQ(device->frame(), 0u);
}

TEST(callback_requests_screen_shot)
{
	auto device = wgfx::Device::create(headless_desc());
	REQUIRE(device != nullptr);
	REQUIRE(device->caps().swapChain);

	const wgfx::FrameBuffer swapChain{
		wgfx::FrameBuffer::create(swap_chain_desc()) };
	REQUIRE(swapChain.valid());
	CHECK(swapChain.is_swap_chain());

	const wgfx::FrameBuffer framebuffer{
		wgfx::FrameBuffer::create(color_framebuffer_desc()) };
	REQUIRE(framebuffer.valid());
	CHECK(!framebuffer.is_swap_chain());

	CHECK(device->request_screen_shot(swapChain, "wgfx-screen-shot"));
	CHECK(!device->request_screen_shot(swapChain, nullptr));
	CHECK(!device->request_screen_shot(swapChain, ""));
	CHECK(!device->request_screen_shot(
		wgfx::FrameBuffer{}, "wgfx-screen-shot"));
	CHECK(!device->request_screen_shot(
		framebuffer, "wgfx-screen-shot"));

	CHECK(device->frame() > 0);

	device->shutdown();
	CHECK(!device->request_screen_shot(
		swapChain, "wgfx-screen-shot"));
}

#else

TEST(callback_requires_bgfx_backend)
{
	CHECK(wgfx::Device::create(headless_desc()) == nullptr);
	CHECK_EQ(static_cast<u8>(wgfx::FatalError::Count), 5u);
}

#endif
