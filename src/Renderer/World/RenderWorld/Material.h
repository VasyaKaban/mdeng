#pragma once

#include "../../DescriptorStorage/DescriptorStorage.h"
#include "../../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
    struct Material
    {
        virtual ~Material();
        virtual void Bind(vk::CommandBuffer command_buffer) const noexcept = 0;
        virtual void WriteDescriptors(DescriptorSetGroup& group) const noexcept = 0;
        virtual bool CompareLess(const Material* mtl) const noexcept = 0;
    };
};
