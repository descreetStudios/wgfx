#pragma once

#include <wgfx/texture.hpp>

#include <wcore/types.hpp>

namespace wgfx
{
	enum class FatalError : u8
	{
		DebugCheck,
		InvalidShader,
		UnableToInitialize,
		UnableToCreateTexture,
		DeviceLost,

		Count,
	};

	class Callback
	{
	public:
		virtual ~Callback() = default;

		virtual void fatal(
			const char* filePath,
			u16 line,
			FatalError code,
			const char* message) = 0;
		virtual void trace(
			const char* filePath,
			u16 line,
			const char* message) = 0;

		virtual void profiler_begin(
			const char* name,
			u32 abgr,
			const char* filePath,
			u16 line)
		{
			(void)name;
			(void)abgr;
			(void)filePath;
			(void)line;
		}

		virtual void profiler_begin_literal(
			const char* name,
			u32 abgr,
			const char* filePath,
			u16 line)
		{
			(void)name;
			(void)abgr;
			(void)filePath;
			(void)line;
		}

		virtual void profiler_end() {}

		// Image data is always 4-byte BGRA and is only valid during the call.
		virtual void screen_shot(
			const char* filePath,
			u32 width,
			u32 height,
			u32 pitch,
			TextureFormat format,
			const void* data,
			u32 size,
			bool yflip)
		{
			(void)filePath;
			(void)width;
			(void)height;
			(void)pitch;
			(void)format;
			(void)data;
			(void)size;
			(void)yflip;
		}

		virtual void capture_begin(
			u32 width,
			u32 height,
			u32 pitch,
			TextureFormat format,
			bool yflip)
		{
			(void)width;
			(void)height;
			(void)pitch;
			(void)format;
			(void)yflip;
		}

		virtual void capture_end() {}

		virtual void capture_frame(const void* data, u32 size)
		{
			(void)data;
			(void)size;
		}
	};
}
