#pragma once

#include <string>
#include "stacktrace_init.h"

namespace hrs
{
	namespace winapi
	{
		class frame
		{
		public:
			using native_symbol_t = PVOID;

			frame(native_symbol_t _symbol = nullptr) noexcept;
			~frame() = default;
			frame(const frame &) = default;
			frame & operator=(const frame &) = default;

			std::string function_name(bool demangle = true) const;
			std::string object_path() const;

			native_symbol_t native_handle() noexcept;
			const native_symbol_t native_handle() const noexcept;

		private:
			std::string get_symbol_info(bool get_only_symbol_name, bool demangle) const;
		private:
			native_symbol_t symbol;
		};
	};
};
