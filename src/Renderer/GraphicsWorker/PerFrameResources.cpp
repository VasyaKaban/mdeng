#include "PerFrameResources.h"
#include "../../hrs/debug.hpp"
#include "../../hrs/scoped_call.hpp"

namespace FireLand
{
	PerFrameResources::PerFrameResources(std::vector<vk::Fence> &&_fences,
										 std::vector<vk::CommandBuffer> &&_command_buffers,
										 std::uint32_t _target_frame_index)
		: fences(std::move(_fences)),
		  command_buffers(std::move(_command_buffers)),
		  target_frame_index(_target_frame_index) {}

	PerFrameResources::PerFrameResources(PerFrameResources &&res) noexcept
		: fences(std::move(res.fences)),
		  command_buffers(std::move(res.command_buffers)),
		  target_frame_index(res.target_frame_index) {}

	PerFrameResources & PerFrameResources::operator=(PerFrameResources &&res) noexcept
	{
		fences = std::move(res.fences);
		command_buffers = std::move(res.command_buffers);
		target_frame_index = res.target_frame_index;
		return *this;
	}

	hrs::expected<PerFrameResources, vk::Result>
	PerFrameResources::Create(vk::Device device, vk::CommandPool pool, std::uint32_t count)
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		hrs::assert_true_debug(pool, "Command pool isn't created yet!");
		hrs::assert_true_debug(count != 0, "Count must be greater than zero!");

		std::vector<vk::Fence> _fences;
		std::vector<vk::CommandBuffer> _command_buffers;

		hrs::scoped_call cleanup([&]()
		{
			for(const auto &fence : _fences)
				device.destroy(fence);

			for(const auto &buf :_command_buffers)
				device.free(pool, buf);
		});

		_fences.reserve(count);
		const vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);
		for(std::uint32_t i = 0; i < count; i++)
		{
			auto [fence_res, fence] = device.createFence(fence_info);
			if(fence_res != vk::Result::eSuccess)
				return fence_res;

			_fences.push_back(fence);
		}

		_command_buffers.reserve(count);
		const vk::CommandBufferAllocateInfo command_buffers_info(pool,
																 vk::CommandBufferLevel::ePrimary,
																 count);

		auto [new_command_buffers_res, new_command_buffers] = device.allocateCommandBuffers(command_buffers_info);
		if(new_command_buffers_res != vk::Result::eSuccess)
			return new_command_buffers_res;

		_command_buffers = std::move(new_command_buffers);

		cleanup.Drop();
		return PerFrameResources(std::move(_fences), std::move(_command_buffers), 0);
	}

	void PerFrameResources::Destroy(vk::Device device, vk::CommandPool pool) noexcept
	{
		if(IsCreated())
		{
			hrs::assert_true_debug(device, "Device isn't created yet!");
			hrs::assert_true_debug(pool, "Command pool isn't created yet!");

			for(const auto &fence : fences)
				device.destroy(fence);

			for(const auto &buf : command_buffers)
				device.free(pool, buf);

			command_buffers.clear();
			fences.clear();
		}
	}

	bool PerFrameResources::IsCreated() const noexcept
	{
		return !command_buffers.empty() && !fences.empty();
	}

	std::uint32_t PerFrameResources::GetCount() const noexcept
	{
		return command_buffers.size();
	}

	void PerFrameResources::UpdateTargetFrameIndex() noexcept
	{
		target_frame_index = (target_frame_index + 1) % GetCount();
	}
};
