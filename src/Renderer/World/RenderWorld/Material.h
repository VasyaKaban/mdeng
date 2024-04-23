#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../DescriptorStorage/DescriptorStorage.h"

namespace FireLand
{
	struct Material
	{
		virtual ~Material();
		virtual void Bind(vk::CommandBuffer command_buffer) const noexcept = 0;
		virtual void WriteDescriptors(DescriptorSetGroup &group) const noexcept = 0;
		virtual bool CompareLess(const Material *mtl) const noexcept = 0;
	};
};
