#include "callback_internal.hpp"

#if defined(WGFX_BACKEND_BGFX)
#include <wcore/logger.hpp>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace wgfx
{
	namespace
	{
#define WGFX_FATAL_MIRROR(name) \
		static_assert(static_cast<u8>(FatalError::name) == bgfx::Fatal::name, \
			"FatalError::" #name " must mirror bgfx::Fatal::" #name)

		WGFX_FATAL_MIRROR(DebugCheck);
		WGFX_FATAL_MIRROR(InvalidShader);
		WGFX_FATAL_MIRROR(UnableToInitialize);
		WGFX_FATAL_MIRROR(UnableToCreateTexture);
		WGFX_FATAL_MIRROR(DeviceLost);
		WGFX_FATAL_MIRROR(Count);

#undef WGFX_FATAL_MIRROR

		std::string format_message(const char* format, va_list args)
		{
			if (!format)
				return {};

			va_list copy;
			va_copy(copy, args);
			const int length{ std::vsnprintf(nullptr, 0, format, copy) };
			va_end(copy);

			if (length <= 0)
				return {};

			std::string message(static_cast<size_t>(length) + 1, '\0');

			va_copy(copy, args);
			std::vsnprintf(message.data(), message.size(), format, copy);
			va_end(copy);

			message.resize(static_cast<size_t>(length));
			return message;
		}
	} // anonymous namespace

	namespace detail
	{
		CallbackAdapter::CallbackAdapter(Callback* callback) noexcept
			: mCallback{ callback }
		{
		}

		void CallbackAdapter::fatal(
			const char* filePath,
			uint16_t line,
			bgfx::Fatal::Enum code,
			const char* message)
		{
			const FatalError fatalCode{ static_cast<FatalError>(code) };

			if (mCallback)
			{
				mCallback->fatal(
					filePath, line, fatalCode, message ? message : "");
				return;
			}

			wcore::logger::fatal(
				"bgfx fatal {}:{}: {} (code {})",
				filePath ? filePath : "",
				line,
				message ? message : "",
				static_cast<u32>(fatalCode));

			std::abort();
		}

		void CallbackAdapter::traceVargs(
			const char* filePath,
			uint16_t line,
			const char* format,
			va_list argList)
		{
			const std::string message{ format_message(format, argList) };

			if (mCallback)
			{
				mCallback->trace(filePath, line, message.c_str());
				return;
			}

			wcore::logger::trace(
				"{}:{}: {}",
				filePath ? filePath : "",
				line,
				message);
		}

		void CallbackAdapter::profilerBegin(
			const char* name,
			uint32_t abgr,
			const char* filePath,
			uint16_t line)
		{
			if (mCallback)
				mCallback->profiler_begin(name, abgr, filePath, line);
		}

		void CallbackAdapter::profilerBeginLiteral(
			const char* name,
			uint32_t abgr,
			const char* filePath,
			uint16_t line)
		{
			if (mCallback)
				mCallback->profiler_begin_literal(name, abgr, filePath, line);
		}

		void CallbackAdapter::profilerEnd()
		{
			if (mCallback)
				mCallback->profiler_end();
		}

		uint32_t CallbackAdapter::cacheReadSize(uint64_t id)
		{
			(void)id;
			return 0;
		}

		bool CallbackAdapter::cacheRead(
			uint64_t id,
			void* data,
			uint32_t size)
		{
			(void)id;
			(void)data;
			(void)size;
			return false;
		}

		void CallbackAdapter::cacheWrite(
			uint64_t id,
			const void* data,
			uint32_t size)
		{
			(void)id;
			(void)data;
			(void)size;
		}

		void CallbackAdapter::screenShot(
			const char* filePath,
			uint32_t width,
			uint32_t height,
			uint32_t pitch,
			bgfx::TextureFormat::Enum format,
			const void* data,
			uint32_t size,
			bool yflip)
		{
			if (mCallback)
			{
				mCallback->screen_shot(filePath,
					width, height, pitch,
					static_cast<TextureFormat>(format),
					data, size, yflip);
				return;
			}

			wcore::logger::warn(
				"bgfx: screen shot '{}' requested without a callback",
				filePath ? filePath : "");
		}

		void CallbackAdapter::captureBegin(
			uint32_t width,
			uint32_t height,
			uint32_t pitch,
			bgfx::TextureFormat::Enum format,
			bool yflip)
		{
			if (mCallback)
			{
				mCallback->capture_begin(
					width, height, pitch,
					static_cast<TextureFormat>(format),
					yflip);
			}
		}

		void CallbackAdapter::captureEnd()
		{
			if (mCallback)
				mCallback->capture_end();
		}

		void CallbackAdapter::captureFrame(
			const void* data, uint32_t size)
		{
			if (mCallback)
				mCallback->capture_frame(data, size);
		}
	} // namespace detail
}
#endif
