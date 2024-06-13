#pragma once

#include <string>

namespace hrs
{
	namespace unwind
	{
		class frame
		{
		public:
			using native_symbol_t = void *;

			frame(native_symbol_t _symbol = nullptr) noexcept;
			~frame() = default;
			frame(const frame &) = default;
			frame & operator=(const frame &) = default;

			std::string function_name(bool demangle = true) const;
			std::string object_path() const;

			native_symbol_t native_handle() noexcept;
			const native_symbol_t native_handle() const noexcept;

		private:
			native_symbol_t symbol;
		};
	};
};
