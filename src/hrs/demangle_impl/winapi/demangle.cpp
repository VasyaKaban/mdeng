#include "demangle.h"
#include <Windows.h>

namespace hrs
{
	namespace winapi
	{
		std::string demangle(const char *mangled_name)
		{
			std::size_t size = 128;
			while(true)
			{
				std::string demangled_name(size, '\0');
				auto count = UnDecorateSymbolName(mangled_name, demangled_name.data(), demangled_name.size(), UNDNAME_COMPLETE);
				if(count == 0)
					return {};

				if(count < demangled_name.size())
					return demangled_name;

				size *= 2;
			}
		}
	};
};
