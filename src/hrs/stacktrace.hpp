#pragma once

#if defined(unix) || defined(__unix) || defined(__unix__)
#    include "stacktrace_impl/unwind/frame.h"
#    include "stacktrace_impl/unwind/stacktrace.h"
#elif defined(_WIN32) || defined(_WIN64)
#    include "stacktrace_impl/winapi/frame.h"
#    include "stacktrace_impl/winapi/stacktrace.h"
#else
#    error Unsupported OS!
#endif
namespace hrs
{
#if defined(unix) || defined(__unix) || defined(__unix__)
    using frame = unwind::frame;
    using stacktrace = unwind::stacktrace;
#elif defined(_WIN32) || defined(_WIN64)
    using frame = winapi::frame;
    using stacktrace = winapi::stacktrace;
#endif
};
