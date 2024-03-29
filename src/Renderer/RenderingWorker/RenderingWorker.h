#pragma once

#include "../../Vulkan/VulkanUtils.hpp"
#include "GBuffer/GBuffer.h"
#include "PostProcessImages/PostProcessImages.h"

namespace FireLand
{
	class RenderingWorker
	{
	public:

		vk::DescriptorSetLayout GetGlobalsDescriptorSetLayout() const noexcept;
		vk::DescriptorSet GetGlobalsDescriptorSet() const noexcept;
		const GBuffer * GetGBuffer() const noexcept;
		ImageFormat GetDefferedPassEvaluationImage() const noexcept;
		bool IsCreated() const noexcept;
		Device * GetDevice() noexcept;
		PostProcessImages * GetPostProcessImages() noexcept;


	private:
	};
};
