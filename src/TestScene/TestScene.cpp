#include "TestScene.h"

#include <iostream>

namespace FireLand
{
	TestScene::~TestScene()
	{

	}

	void TestScene::Render(vk::CommandBuffer command_buffer,
						   vk::RenderPass renderpass,
						   std::uint32_t frame_index,
						   vk::DescriptorSet globals_set,
						   vk::DescriptorSet renderpass_set,
						   std::uint32_t subpass_index,
						   const ComputedCamera &camera) const noexcept
	{
		const auto &target_shaders = shaders[subpass_index];
		for(const auto &shader : target_shaders)
		{
			if(!shader->IsSceneIndependent())
				continue;

			shader->Bind(command_buffer, globals_set, renderpass_set);

			command_buffer.setViewport(0, camera.GetCamera()->GetViewport());
			command_buffer.setScissor(0, camera.GetCamera()->GetScissorsRect());



			if(subpass_index == DefferedPass::SubpassIndices::RasterizationSubpassIndex)
			{
				auto id_mat = hrs::math::glsl::std430::mat4x4::identity();
				id_mat[2][3] = 1;
				id_mat[3][3] = 0;
				const std::byte *data_ptr = reinterpret_cast<const std::byte *>(&id_mat);
				shader->PushConstants(command_buffer, data_ptr);
			}

			if(subpass_index == DefferedPass::SubpassIndices::RasterizationSubpassIndex)
				command_buffer.draw(3, 1, 0, 0);
			else
				command_buffer.draw(6, 1, 0, 0);
		}
	}
};
