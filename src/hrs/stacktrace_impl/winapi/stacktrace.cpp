#include "stacktrace.h"
#include "frame.h"
#include "stacktrace_init.h"
#include <fstream>

namespace hrs
{
    namespace winapi
    {
        stacktrace::stacktrace(std::size_t capture_frame_count, std::size_t skip_frame_count)
        {
            std::vector<PVOID> raw_frames(capture_frame_count);

            auto captured_count = RtlCaptureStackBackTrace(skip_frame_count + 1,
                                                           capture_frame_count,
                                                           raw_frames.data(),
                                                           nullptr);

            std::vector<frame> _frames;
            _frames.reserve(captured_count);
            for(std::size_t i = 0; i < captured_count; i++)
                _frames.emplace_back(raw_frames[i]);

            frames = std::move(_frames);
        }

        stacktrace::container_t::iterator stacktrace::begin() noexcept
        {
            return frames.begin();
        }

        stacktrace::container_t::iterator stacktrace::end() noexcept
        {
            return frames.end();
        }

        stacktrace::container_t::const_iterator stacktrace::begin() const noexcept
        {
            return frames.begin();
        }

        stacktrace::container_t::const_iterator stacktrace::end() const noexcept
        {
            return frames.end();
        }

        frame& stacktrace::operator[](std::size_t index) noexcept
        {
            return frames[index];
        }

        const frame& stacktrace::operator[](std::size_t index) const noexcept
        {
            return frames[index];
        }

        std::size_t stacktrace::size() const noexcept
        {
            return frames.size();
        }

        bool stacktrace::empty() const noexcept
        {
            return frames.empty();
        }

        bool stacktrace::dump(const std::filesystem::path& path) const noexcept
        {
            if(frames.empty())
                return false;

            std::ofstream fd;
            fd.open(path, std::ios::trunc);
            if(!fd.is_open())
                return false;

            fd.write(reinterpret_cast<const char*>(frames.data()), sizeof(frame) * frames.size());
            return true;
        }
    };
};
