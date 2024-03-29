#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/expected.hpp"

namespace FireLand
{
	class Device;

	class PerFrameResources : public hrs::non_copyable
	{
		void init(vk::CommandPool _command_pool,
				  std::vector<vk::CommandBuffer> &&_command_buffers,
				  vk::DescriptorPool _globals_descriptor_pool,
				  vk::DescriptorSetLayout _globals_descriptor_set_layout,
				  std::vector<vk::DescriptorSet> &&_globals_descriptor_sets,
				  std::vector<vk::Fence> &&_fences) noexcept;

	public:
		PerFrameResources(const Device *parent_device) noexcept;
		~PerFrameResources();
		PerFrameResources(PerFrameResources &&res) noexcept;
		PerFrameResources & operator=(PerFrameResources &&res) noexcept;

		vk::Result Recreate(std::uint32_t queue_family_index, std::uint32_t count);

		void Destroy() noexcept;
		bool IsCreated() const noexcept;

		std::uint32_t GetCount() const noexcept;

		const Device * GetParentDevice() const noexcept;

		vk::CommandBuffer GetTargetCommandBuffer() const noexcept;
		vk::DescriptorSet GetTargetGlobalsDescriptorSet() const noexcept;
		vk::Fence GetTargetFence() const noexcept;

		vk::DescriptorSetLayout GetGlobalsDescriptorSetLayout() const noexcept;
		std::uint32_t GetTargetFrameIndex() const noexcept;
		void UpdateTargetFrameIndex() noexcept;
		void ResetTargetFrameIndex() noexcept;

	private:
		const Device *parent_device;

		vk::CommandPool command_pool;
		std::vector<vk::CommandBuffer> command_buffers;

		vk::DescriptorPool globals_descriptor_pool;
		vk::DescriptorSetLayout globals_descriptor_set_layout;
		std::vector<vk::DescriptorSet> globals_descriptor_sets;

		std::vector<vk::Fence> fences;

		std::uint32_t target_frame_index;
	};
};
