#pragma once

#include "VulkanInclude.h"
#include <string_view>
#include <variant>

namespace FireLand
{
    class InitResult
    {
    public:
        InitResult() = default;
        InitResult(std::string_view failed_name) noexcept;
        InitResult(VkResult vk_result) noexcept;

        template<std::size_t N>
        InitResult(const char (&failed_name)[N]) noexcept
            : InitResult(std::string_view{failed_name})
        {}

        ~InitResult() = default;
        InitResult(const InitResult&) = default;
        InitResult(InitResult&&) = default;
        InitResult& operator=(const InitResult&) = default;
        InitResult& operator=(InitResult&&) = default;
        InitResult& operator=(VkResult vk_result) noexcept;
        InitResult& operator=(std::string_view failed_name) noexcept;

        template<std::size_t N>
        InitResult& operator=(const char (&failed_name)[N]) noexcept
        {
            result = std::string_view{failed_name};
            return *this;
        }

        bool IsRequiredFailure() const noexcept;
        bool IsVulkanResult() const noexcept;

        std::string_view GetRequiredName() const noexcept;
        VkResult GetVulkanResult() const noexcept;
    private:
        std::variant<std::string_view, VkResult> result;
    };
};
