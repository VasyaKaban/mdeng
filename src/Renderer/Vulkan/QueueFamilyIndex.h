#pragma once

#include "VulkanInclude.h"
#include <cstdint>

namespace FireLand
{
    struct QueueFamilyIndex
    {
        VkQueue queue;
        std::uint32_t family_index;
    };
};
