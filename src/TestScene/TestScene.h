#pragma once

#include "../Scene/Scene.h"
#include "../Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPass.h"
#include "../Renderer/GraphicsWorker/RenderPass/DefferedPass/DefferedPassShader.h"

namespace FireLand
{
	class TestScene : public Scene
	{
	public:
		TestScene() = default;
		virtual ~TestScene() override;

		virtual void Render(vk::CommandBuffer command_buffer,
							vk::RenderPass renderpass,
							std::uint32_t frame_index,
							vk::DescriptorSet globals_set,
							vk::DescriptorSet renderpass_set,
							std::uint32_t subpass_index,
							const ComputedCamera &camera) const noexcept override;


		std::array<std::vector<DefferedPassShader *>, DefferedPass::SubpassIndices::LastUnusedSubpassIndex> shaders;
	};
};
