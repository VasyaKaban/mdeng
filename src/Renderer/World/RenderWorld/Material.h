#pragma once

#include "../../../hrs/hint_cast_object.hpp"
#include "../../../Vulkan/VulkanInclude.hpp"
#include "../../DescriptorStorage/DescriptorStorage.h"

namespace FireLand
{
	struct Material
	{
		virtual ~Material();
		virtual void Bind(vk::CommandBuffer command_buffer) const noexcept = 0;
		virtual bool DescriptorSetPerFrame() const noexcept = 0;
		virtual void WriteDescriptors(DescriptorSetGroup &group) const noexcept = 0;
		virtual hrs::hint_cast_object::hint_func_type GetHint() const noexcept = 0;
		virtual bool CompareLess(const Material *mtl) const noexcept = 0;
	};
};
