#pragma once

#include <string_view>
#include <variant>

namespace FireLand
{
    class LoaderInitResult
    {
    public:
        LoaderInitResult() noexcept;
        LoaderInitResult(std::monostate) noexcept;
        LoaderInitResult(std::size_t loaded_count) noexcept;
        LoaderInitResult(std::string_view failed_name) noexcept;

        template<std::size_t N>
        LoaderInitResult(const char (&failed_name)[N]) noexcept
            : LoaderInitResult(std::string_view{failed_name})
        {}

        ~LoaderInitResult() = default;
        LoaderInitResult(const LoaderInitResult&) = default;
        LoaderInitResult(LoaderInitResult&&) = default;
        LoaderInitResult& operator=(const LoaderInitResult&) = default;
        LoaderInitResult& operator=(LoaderInitResult&&) = default;

        bool IsOk() const noexcept;
        bool IsFailure() const noexcept;

        bool IsLoaderFunctionFailure() const noexcept;
        bool IsRequiredFunctionFailure() const noexcept;

        std::size_t GetLoadedCount() const noexcept;
        std::string_view GetRequiredFailureName() const noexcept;
    private:
        std::variant<std::size_t, std::string_view, std::monostate> result;
    };
};
