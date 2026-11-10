#pragma once

#include <wgfx/callback.hpp>

#if defined(WGFX_BACKEND_BGFX)
#include <bgfx/bgfx.h>

#include <cstdarg>

namespace wgfx
{
	namespace detail
	{
		class CallbackAdapter final : public bgfx::CallbackI
		{
		public:
			explicit CallbackAdapter(Callback* callback) noexcept;

			void fatal(
				const char* filePath,
				uint16_t line,
				bgfx::Fatal::Enum code,
				const char* message) override;
			void traceVargs(
				const char* filePath,
				uint16_t line,
				const char* format,
				va_list argList) override;

			void profilerBegin(
				const char* name,
				uint32_t abgr,
				const char* filePath,
				uint16_t line) override;
			void profilerBeginLiteral(
				const char* name,
				uint32_t abgr,
				const char* filePath,
				uint16_t line) override;
			void profilerEnd() override;

			uint32_t cacheReadSize(uint64_t id) override;
			bool cacheRead(uint64_t id, void* data, uint32_t size) override;
			void cacheWrite(uint64_t id, const void* data, uint32_t size) override;

			void screenShot(
				const char* filePath,
				uint32_t width,
				uint32_t height,
				uint32_t pitch,
				bgfx::TextureFormat::Enum format,
				const void* data,
				uint32_t size,
				bool yflip) override;

			void captureBegin(
				uint32_t width,
				uint32_t height,
				uint32_t pitch,
				bgfx::TextureFormat::Enum format,
				bool yflip) override;
			void captureEnd() override;
			void captureFrame(const void* data, uint32_t size) override;

		private:
			Callback* mCallback{ nullptr };
		};
	} // namespace detail
}
#else
namespace wgfx
{
	namespace detail
	{
		class CallbackAdapter
		{
		public:
			explicit CallbackAdapter(Callback*) noexcept {}
		};
	} // namespace detail
}
#endif
