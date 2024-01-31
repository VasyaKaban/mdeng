#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/expected.hpp"

namespace FireLand
{
	class PerFrameResources : public hrs::non_copyable
	{
		void init(vk::CommandPool _command_pool,
				  std::vector<vk::Fence> &&_fences,
				  std::vector<vk::CommandBuffer> &&_command_buffers,
				  std::uint32_t _target_frame_index) noexcept;

	public:
		PerFrameResources(vk::Device _parent_device) noexcept;
		~PerFrameResources();
		PerFrameResources(PerFrameResources &&res) noexcept;
		PerFrameResources & operator=(PerFrameResources &&res) noexcept;

		vk::Result Recreate(std::uint32_t queue_family_index, std::uint32_t count);

		void Destroy() noexcept;
		bool IsCreated() const noexcept;
		std::uint32_t GetCount() const noexcept;
		const std::vector<vk::Fence> & GetFences() const noexcept;
		vk::Fence GetTargetFence() const noexcept;
		const std::vector<vk::CommandBuffer> & GetCommandBuffers() const noexcept;
		vk::CommandBuffer GetTargetCommandBuffer() const noexcept;
		std::uint32_t GetTargetFrameIndex() const noexcept;
		void UpdateTargetFrameIndex() noexcept;


	private:
		vk::Device parent_device;
		vk::CommandPool command_pool;
		std::vector<vk::Fence> fences;
		std::vector<vk::CommandBuffer> command_buffers;
		std::uint32_t target_frame_index;
	};
};
