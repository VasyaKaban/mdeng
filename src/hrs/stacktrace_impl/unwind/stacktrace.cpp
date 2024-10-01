#include "stacktrace.h"
#include "frame.h"
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <unwind.h>

namespace hrs
{
    namespace unwind
    {
        struct unwind_data
        {
            std::size_t capture_frame_count;
            std::size_t skip_frame_count;
            std::vector<frame>& frames;
        };

        _Unwind_Reason_Code unwind_callback(_Unwind_Context* context, void* data) noexcept
        {
            unwind_data* _data = reinterpret_cast<unwind_data*>(data);
            if(_data->capture_frame_count == 0)
                return _Unwind_Reason_Code::_URC_END_OF_STACK;

            if(_data->skip_frame_count != 0)
            {
                _data->skip_frame_count--;
                auto ip = _Unwind_GetIP(context);
                if(ip != 0)
                    return _Unwind_Reason_Code::_URC_NO_REASON;

                return _Unwind_Reason_Code::_URC_END_OF_STACK;
            }

            auto ip = _Unwind_GetIP(context);
            if(!ip)
                return _Unwind_Reason_Code::_URC_END_OF_STACK;

            _data->frames.emplace_back(reinterpret_cast<frame::native_symbol_t>(ip));
            _data->capture_frame_count--;

            return _Unwind_Reason_Code::_URC_NO_REASON;
        }

        stacktrace::stacktrace(std::size_t capture_frame_count, std::size_t skip_frame_count)
        {
            std::vector<frame> _frames;
            _frames.reserve(capture_frame_count);

            unwind_data unwind_data{capture_frame_count, skip_frame_count + 1, _frames};
            _Unwind_Backtrace(unwind_callback, reinterpret_cast<void*>(&unwind_data));

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

            int fd = open(path.c_str(), O_SYNC | O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if(fd == -1)
                return false;

            int write_res = write(fd, frames.data(), sizeof(void*) * frames.size());

            int close_res = close(fd);
            if(close_res != 0)
                return false;

            return write_res != -1;
        }
    };
};
