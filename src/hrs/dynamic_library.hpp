#pragma once

#if defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__) || defined(__MACH__) || defined(macintosh) || defined(Macintosh)
#include "dynamic_library_impl/dl/dynamic_library.h"
#elif defined(_WIN32) || defined(_WIN64)
#include "dynamic_library_impl/winapi/dynamic_library.h"
#else
#error Unsupported OS!
#endif
namespace hrs
{
#if defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__) || defined(__MACH__) || defined(macintosh) || defined(Macintosh)
	using dynamic_library = dl::dynamic_library;
#elif defined(_WIN32) || defined(_WIN64)
	using dynamic_library = winapi::dynamic_library;
#endif
};
