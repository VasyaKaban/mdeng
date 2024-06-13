#include "frame.h"
#include <cstdlib>
#include <cstring>
#include "../../demangle.hpp"
#include "../../scoped_call.hpp"

namespace hrs
{
	namespace winapi
	{
		frame::frame(native_symbol_t _symbol) noexcept
			: symbol(_symbol) {}

		std::string frame::function_name(bool demangle) const
		{
			if(!symbol)
				return "";

			return get_symbol_info(true);
		}

		std::string frame::object_path() const
		{
			if(!symbol)
				return "";

			return get_symbol_info(false);
		}

		frame::native_symbol_t frame::native_handle() noexcept
		{
			return symbol;
		}

		const frame::native_symbol_t frame::native_handle() const noexcept
		{
			return symbol;
		}

		std::string get_symbol_info(bool get_only_symbol_name, bool demangle)
		{
			auto process = GetCurrentProcess();

			bool sym_inited = false;
			bool options_replaced = false;
			PSYMBOL_INFO symbol_info = nullptr;
			hrs::scoped_call cleanup = [process]()
			{
				if(symbol_info)
					std::free(symbol_info);

				if(options_replaced)
				{
					auto options = SymGetOptions();
					SymSetOptions(options | SYMOPT_UNDNAME);
				}

				if(sym_inited)
					SymCleanup(process);
			};

			sym_inited = SymInitialize(process, nullptr, TRUE);
			if(!sym_inited)
				return "";

			if(get_only_symbol_name && !demangle)
			{
				auto options = SymGetOptions();
				SymSetOptions(options & ~SYMOPT_UNDNAME);
				options_replaced = true;
			}

			if(get_only_symbol_name)
				symbol_info = std::calloc(sizeof(SYMBOL_INFO) + sizeof(char) * MAX_SYM_NAME + 1, 1);
			else
				symbol_info = std::calloc(sizeof(SYMBOL_INFO), 1);

			if(!symbol_info)
				return "";

			symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);
			symbol_info->MaxNameLen = (get_only_name ? 0 : MAX_SYM_NAME);

			DRORD64 addr = *reinterpret_cast<DWORD64 *>(_symbol);
			auto res = SymFromAddr(process, addr, 0, symbol_info);
			if(res == FALSE)
				return "";

			if(get_only_symbol_name)
				std::string(symbol_info->name, std::strlen(symbol_info->name);
			else
			{
				HMODULE module_addr;
				auto get_module_res = GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCWSTR>(symbol_info->ModBase), &module_addr);
				if(!get_module_res)
					return "";

				std::string module_name(MAX_PATH, '\0');
				GetModuleFileName(module_addr, module_name.data(), module_name.size());
				return module_name;
			}
		}
	};
};
