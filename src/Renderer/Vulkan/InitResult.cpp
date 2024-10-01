#include "InitResult.h"

namespace FireLand
{
    InitResult::InitResult(std::string_view failed_name) noexcept
        : result(failed_name)
    {}

    InitResult::InitResult(VkResult vk_result) noexcept
        : result(vk_result)
    {}

    InitResult& InitResult::operator=(VkResult vk_result) noexcept
    {
        result = vk_result;
        return *this;
    }

    InitResult& InitResult::operator=(std::string_view failed_name) noexcept
    {
        result = failed_name;
        return *this;
    }

    bool InitResult::IsRequiredFailure() const noexcept
    {
        return std::holds_alternative<std::string_view>(result);
    }

    bool InitResult::IsVulkanResult() const noexcept
    {
        return std::holds_alternative<VkResult>(result);
    }

    std::string_view InitResult::GetRequiredName() const noexcept
    {
        return std::get<std::string_view>(result);
    }

    VkResult InitResult::GetVulkanResult() const noexcept
    {
        return std::get<VkResult>(result);
    }
};
