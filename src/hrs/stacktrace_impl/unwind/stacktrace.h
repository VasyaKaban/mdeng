#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace hrs
{
    namespace unwind
    {
        class frame;

        class stacktrace
        {
        public:
            using container_t = std::vector<frame>;

            stacktrace(std::size_t capture_frame_count, std::size_t skip_frame_count);
            ~stacktrace() = default;
            stacktrace(const stacktrace& us) = default;
            stacktrace& operator=(const stacktrace& us) = default;
            stacktrace(stacktrace&& us) = default;
            stacktrace& operator=(stacktrace&& us) = default;

            container_t::iterator begin() noexcept;
            container_t::iterator end() noexcept;
            container_t::const_iterator begin() const noexcept;
            container_t::const_iterator end() const noexcept;

            frame& operator[](std::size_t index) noexcept;
            const frame& operator[](std::size_t index) const noexcept;

            std::size_t size() const noexcept;
            bool empty() const noexcept;

            bool dump(const std::filesystem::path& path) const noexcept;
        private:
            container_t frames;
        };
    };
};
