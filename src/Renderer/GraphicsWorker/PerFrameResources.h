#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/expected.hpp"

namespace FireLand
{
	struct PerFrameResources : public hrs::non_copyable
	{
		PerFrameResources(std::vector<vk::Fence> &&_fences,
						  std::vector<vk::CommandBuffer> &&_command_buffers,
						  std::uint32_t _target_frame_index);

		PerFrameResources() = default;
		~PerFrameResources() = default;
		PerFrameResources(PerFrameResources &&res) noexcept;
		PerFrameResources & operator=(PerFrameResources &&res) noexcept;

		static hrs::expected<PerFrameResources, vk::Result> Create(vk::Device device,
																   vk::CommandPool pool,
																   std::uint32_t count);

		void Destroy(vk::Device device, vk::CommandPool pool) noexcept;
		bool IsCreated() const noexcept;
		std::uint32_t GetCount() const noexcept;
		void UpdateTargetFrameIndex() noexcept;

		std::vector<vk::Fence> fences;
		std::vector<vk::CommandBuffer> command_buffers;
		std::uint32_t target_frame_index;
	};
};
