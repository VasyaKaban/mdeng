#include "PerFrameResources.h"
#include "../../hrs/debug.hpp"
#include "../../hrs/scoped_call.hpp"

namespace FireLand
{
	void PerFrameResources::init(vk::CommandPool _command_pool,
								 std::vector<vk::Fence> &&_fences,
								 std::vector<vk::CommandBuffer> &&_command_buffers,
								 std::uint32_t _target_frame_index) noexcept
	{
		command_pool = _command_pool;
		fences = std::move(_fences);
		command_buffers = std::move(_command_buffers);
		target_frame_index = _target_frame_index;
	}

	PerFrameResources::PerFrameResources(vk::Device _parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(parent_device, "Parent device isn't created yet!");
	}

	PerFrameResources::~PerFrameResources()
	{
		Destroy();
	}

	PerFrameResources::PerFrameResources(PerFrameResources &&res) noexcept
		: parent_device(res.parent_device),
		  command_pool(res.command_pool),
		  fences(std::move(res.fences)),
		  command_buffers(std::move(res.command_buffers)),
		  target_frame_index(res.target_frame_index)
	{
		res.command_pool = VK_NULL_HANDLE;
	}

	PerFrameResources & PerFrameResources::operator=(PerFrameResources &&res) noexcept
	{
		Destroy();

		parent_device = res.parent_device;
		command_pool = res.command_pool;
		fences = std::move(res.fences);
		command_buffers = std::move(res.command_buffers);
		target_frame_index = res.target_frame_index;

		res.command_pool = VK_NULL_HANDLE;
		return *this;
	}

	vk::Result
	PerFrameResources::Recreate(std::uint32_t queue_family_index, std::uint32_t count)
	{
		hrs::assert_true_debug(count != 0, "Count must be greater than zero!");

		Destroy();

		std::vector<vk::Fence> _fences;
		std::vector<vk::CommandBuffer> _command_buffers;

		const vk::CommandPoolCreateInfo command_pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
														  vk::CommandPoolCreateFlagBits::eTransient,
														  queue_family_index);

		auto [u_command_pool_res, u_command_pool] = parent_device.createCommandPoolUnique(command_pool_info);
		if(u_command_pool_res != vk::Result::eSuccess)
			return u_command_pool_res;

		hrs::scoped_call cleanup([&]()
		{
			for(const auto &fence : _fences)
				parent_device.destroy(fence);

			for(const auto &buf :_command_buffers)
				parent_device.free(u_command_pool.get(), buf);
		});

		_fences.reserve(count);
		const vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);
		for(std::uint32_t i = 0; i < count; i++)
		{
			auto [fence_res, fence] = parent_device.createFence(fence_info);
			if(fence_res != vk::Result::eSuccess)
				return fence_res;

			_fences.push_back(fence);
		}

		_command_buffers.reserve(count);
		const vk::CommandBufferAllocateInfo command_buffers_info(u_command_pool.get(),
																 vk::CommandBufferLevel::ePrimary,
																 count);

		auto [new_command_buffers_res, new_command_buffers] =
			parent_device.allocateCommandBuffers(command_buffers_info);
		if(new_command_buffers_res != vk::Result::eSuccess)
			return new_command_buffers_res;

		_command_buffers = std::move(new_command_buffers);

		cleanup.Drop();
		init(u_command_pool.release(),
			 std::move(_fences),
			 std::move(_command_buffers),
			 0);

		return vk::Result::eSuccess;
	}

	void PerFrameResources::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		for(const auto fence : fences)
			parent_device.destroy(fence);

		for(const auto buf : command_buffers)
			parent_device.free(command_pool, buf);

		command_buffers.clear();
		fences.clear();
		parent_device.destroy(command_pool);

		command_pool = VK_NULL_HANDLE;
	}

	bool PerFrameResources::IsCreated() const noexcept
	{
		return command_pool;
	}

	std::uint32_t PerFrameResources::GetCount() const noexcept
	{
		return command_buffers.size();
	}

	const std::vector<vk::Fence> & PerFrameResources::GetFences() const noexcept
	{
		return fences;
	}

	vk::Fence PerFrameResources::GetTargetFence() const noexcept
	{
		return fences[target_frame_index];
	}

	const std::vector<vk::CommandBuffer> & PerFrameResources::GetCommandBuffers() const noexcept
	{
		return command_buffers;
	}

	vk::CommandBuffer PerFrameResources::GetTargetCommandBuffer() const noexcept
	{
		return command_buffers[target_frame_index];
	}

	std::uint32_t PerFrameResources::GetTargetFrameIndex() const noexcept
	{
		return target_frame_index;
	}

	void PerFrameResources::UpdateTargetFrameIndex() noexcept
	{
		target_frame_index = (target_frame_index + 1) % GetCount();
	}
};
