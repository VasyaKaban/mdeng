#include "PerFrameResources.h"
#include "../Context/Device.h"
#include "../../hrs/debug.hpp"
#include "../../hrs/scoped_call.hpp"

namespace FireLand
{
	void PerFrameResources::init(vk::CommandPool _command_pool,
								 std::vector<vk::CommandBuffer> &&_command_buffers,
								 vk::DescriptorPool _globals_descriptor_pool,
								 vk::DescriptorSetLayout _globals_descriptor_set_layout,
								 std::vector<vk::DescriptorSet> &&_globals_descriptor_sets,
								 std::vector<vk::Fence> &&_fences) noexcept
	{
		command_pool = _command_pool;
		command_buffers = std::move(_command_buffers);

		globals_descriptor_pool = _globals_descriptor_pool;
		globals_descriptor_set_layout = _globals_descriptor_set_layout;
		globals_descriptor_sets = std::move(_globals_descriptor_sets);

		fences = std::move(_fences);

		target_frame_index = 0;
	}

	PerFrameResources::PerFrameResources(const Device *_parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device pointer points to null!");
		hrs::assert_true_debug(_parent_device->GetHandle(), "Parent device isn't created yet!");
	}

	PerFrameResources::~PerFrameResources()
	{
		Destroy();
	}

	PerFrameResources::PerFrameResources(PerFrameResources &&res) noexcept
		: parent_device(res.parent_device),
		  command_pool(std::exchange(res.command_pool, VK_NULL_HANDLE)),
		  command_buffers(std::move(res.command_buffers)),
		  globals_descriptor_pool(std::exchange(res.globals_descriptor_pool, VK_NULL_HANDLE)),
		  globals_descriptor_set_layout(std::exchange(res.globals_descriptor_set_layout, VK_NULL_HANDLE)),
		  globals_descriptor_sets(std::move(res.globals_descriptor_sets)),
		  fences(std::move(res.fences)),
		  target_frame_index(std::exchange(res.target_frame_index, 0)) {}

	PerFrameResources & PerFrameResources::operator=(PerFrameResources &&res) noexcept
	{
		Destroy();

		parent_device = res.parent_device;
		command_pool = std::exchange(res.command_pool, VK_NULL_HANDLE);
		command_buffers = std::move(res.command_buffers);
		globals_descriptor_pool = std::exchange(res.globals_descriptor_pool, VK_NULL_HANDLE);
		globals_descriptor_set_layout = std::exchange(res.globals_descriptor_set_layout, VK_NULL_HANDLE);
		globals_descriptor_sets = std::move(res.globals_descriptor_sets);
		fences = std::move(res.fences);
		target_frame_index = std::exchange(res.target_frame_index, 0);

		return *this;
	}

	vk::Result PerFrameResources::Recreate(std::uint32_t queue_family_index, std::uint32_t count)
	{
		hrs::assert_true_debug(count != 0, "Count must be greater than zero!");

		Destroy();

		vk::Device device_handle = parent_device->GetHandle();
		const std::array pool_sizes =
		{
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1 * count)
		};

		const vk::DescriptorPoolCreateInfo descriptor_pool_info({},
																count,
																pool_sizes);

		auto [u_descriptor_pool_res, u_descriptor_pool] =
			device_handle.createDescriptorPoolUnique(descriptor_pool_info);
		if(u_descriptor_pool_res != vk::Result::eSuccess)
			return u_descriptor_pool_res;

		const std::array descriptor_set_layout_bindings =
		{
			vk::DescriptorSetLayoutBinding(0,
										   vk::DescriptorType::eUniformBuffer,
										   1,
										   vk::ShaderStageFlagBits::eAll,
										   {})
		};

		const vk::DescriptorSetLayoutCreateInfo descriptor_set_layout_info({},
																		   descriptor_set_layout_bindings);

		auto [u_descriptor_set_layout_res, u_descriptor_set_layout] =
			device_handle.createDescriptorSetLayoutUnique(descriptor_set_layout_info);
		if(u_descriptor_set_layout_res != vk::Result::eSuccess)
			return u_descriptor_set_layout_res;

		const std::vector descriptor_set_layouts(count, u_descriptor_set_layout.get());
		const vk::DescriptorSetAllocateInfo descriptor_set_allocate_info(u_descriptor_pool.get(),
																		 descriptor_set_layouts);

		auto [_descriptor_sets_res, _descriptor_sets] =
			device_handle.allocateDescriptorSets(descriptor_set_allocate_info);
		if(_descriptor_sets_res != vk::Result::eSuccess)
			return _descriptor_sets_res;

		const vk::DescriptorPool reset_descriptor_pool = u_descriptor_pool.get();
		std::vector<vk::Fence> _fences;
		hrs::scoped_call per_frame_resources_dtor([&]()
		{
			for(const auto fence : _fences)
				device_handle.destroy(fence);

			device_handle.resetDescriptorPool(reset_descriptor_pool);
		});

		const vk::CommandPoolCreateInfo command_pool_info(vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
														  vk::CommandPoolCreateFlagBits::eTransient,
														  queue_family_index);

		auto [u_command_pool_res, u_command_pool] =
			device_handle.createCommandPoolUnique(command_pool_info);
		if(u_command_pool_res != vk::Result::eSuccess)
			return u_command_pool_res;

		_fences.reserve(count);
		const vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);
		for(std::uint32_t i = 0; i < count; i++)
		{
			auto [fence_res, fence] = device_handle.createFence(fence_info);
			if(fence_res != vk::Result::eSuccess)
				return fence_res;

			_fences.push_back(fence);
		}

		const vk::CommandBufferAllocateInfo command_buffers_info(u_command_pool.get(),
																 vk::CommandBufferLevel::ePrimary,
																 count);

		auto [_command_buffers_res, _command_buffers] =
			device_handle.allocateCommandBuffers(command_buffers_info);
		if(_command_buffers_res != vk::Result::eSuccess)
			return _command_buffers_res;


		per_frame_resources_dtor.Drop();
		init(u_command_pool.release(),
			 std::move(_command_buffers),
			 u_descriptor_pool.release(),
			 u_descriptor_set_layout.release(),
			 std::move(_descriptor_sets),
			 std::move(_fences));

		return vk::Result::eSuccess;
	}

	void PerFrameResources::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		vk::Device device_handle = parent_device->GetHandle();
		for(const auto fence : fences)
			device_handle.destroy(fence);

		device_handle.resetDescriptorPool(globals_descriptor_pool);
		device_handle.destroy(globals_descriptor_set_layout);
		device_handle.destroy(globals_descriptor_pool);

		for(const auto buf : command_buffers)
			device_handle.free(command_pool, buf);

		device_handle.destroy(command_pool);

		fences.clear();

		globals_descriptor_sets.clear();
		globals_descriptor_set_layout = VK_NULL_HANDLE;
		globals_descriptor_pool = VK_NULL_HANDLE;

		command_buffers.clear();
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

	const Device * PerFrameResources::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	vk::CommandBuffer PerFrameResources::GetTargetCommandBuffer() const noexcept
	{
		return command_buffers[target_frame_index];
	}

	vk::DescriptorSet PerFrameResources::GetTargetGlobalsDescriptorSet() const noexcept
	{
		return globals_descriptor_sets[target_frame_index];
	}

	vk::Fence PerFrameResources::GetTargetFence() const noexcept
	{
		return fences[target_frame_index];
	}

	vk::DescriptorSetLayout PerFrameResources::GetGlobalsDescriptorSetLayout() const noexcept
	{
		return globals_descriptor_set_layout;
	}

	std::uint32_t PerFrameResources::GetTargetFrameIndex() const noexcept
	{
		return target_frame_index;
	}

	void PerFrameResources::UpdateTargetFrameIndex() noexcept
	{
		target_frame_index = (target_frame_index + 1) % GetCount();
	}

	void PerFrameResources::ResetTargetFrameIndex() noexcept
	{
		target_frame_index = 0;
	}
};
