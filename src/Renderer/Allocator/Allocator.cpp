#include "Allocator.h"
#include "MemoryPool.h"

namespace FireLand
{
	Allocator::Allocator(vk::Device _device, vk::PhysicalDevice ph_device)
		: device(_device)
	{
		hrs::assert_true_debug(_device, "Device isn't created yet!");
		hrs::assert_true_debug(ph_device, "Physical device isn't created yet!");

		vk::DeviceSize buffer_image_granularity = ph_device.getProperties().limits.bufferImageGranularity;
		auto memory_props = ph_device.getMemoryProperties();
		memory_types.reserve(memory_props.memoryTypeCount);
		for(std::size_t i = 0; i < memory_props.memoryTypeCount; i++)
		{
			const auto &mem_type = memory_props.memoryTypes[i];
			memory_types.emplace_back(device,
									  memory_props.memoryHeaps[mem_type.heapIndex],
									  mem_type.propertyFlags,
									  i,
									  buffer_image_granularity);
		}
	}

	Allocator::~Allocator()
	{
		Destroy();
	}

	Allocator::Allocator(Allocator &&allocator) noexcept
		: device(allocator.device),
		  memory_types(std::move(allocator.memory_types)) {}

	Allocator & Allocator::operator=(Allocator &&allocator) noexcept
	{
		Destroy();

		device = allocator.device;
		memory_types = std::move(allocator.memory_types);

		return *this;
	}

	void Allocator::Destroy() noexcept
	{
		memory_types.clear();
	}

	hrs::expected<BoundedBuffer, hrs::error>
	Allocator::Create(const vk::BufferCreateInfo &info,
					  MemoryTypeSatisfyOp op,
					  vk::MemoryPropertyFlags desired_props,
					  vk::DeviceSize alignment,
					  hrs::flags<AllocationFlags> flags,
					  const std::function<NewPoolSizeCalculator> &calc)
	{
#warning NO SUPPORT FOR SPARSE RESOURCES AND LINUX DRM FORMAT! ONLY PLAIN RESOURCES!

		hrs::assert_true_debug(hrs::is_power_of_two(alignment), "Alignment is not power of two!");

		auto [u_buffer_res, u_buffer] = device.createBufferUnique(info);
		if(u_buffer_res != vk::Result::eSuccess)
			return {u_buffer_res};

		hrs::error common_err = AllocatorResult::NoSatisfiedMemoryTypes;
		auto req = device.getBufferMemoryRequirements(u_buffer.get());
		if(req.alignment < alignment)
			req.alignment = alignment;

		for(auto mem_type_it = memory_types.begin(); mem_type_it != memory_types.end(); mem_type_it++)
		{
			if(!is_memory_type_satisfy(*mem_type_it, op, desired_props, req))
				continue;

			auto block_pool_bind_exp = mem_type_it->Bind(u_buffer.get(), req, flags, calc);
			if(block_pool_bind_exp)
				return BoundedBuffer(std::move(block_pool_bind_exp.value()), mem_type_it, u_buffer.release());
			else if(is_non_terminate_error(block_pool_bind_exp.error()))
				common_err = block_pool_bind_exp.error();
			else
				return {block_pool_bind_exp.error()};
		}

		return common_err;
	}

	hrs::expected<BoundedImage, hrs::error>
	Allocator::Create(const vk::ImageCreateInfo &info,
					  MemoryTypeSatisfyOp op,
					  vk::MemoryPropertyFlags desired_props,
					  vk::DeviceSize alignment,
					  hrs::flags<AllocationFlags> flags,
					  const std::function<NewPoolSizeCalculator> &calc)
	{
		hrs::assert_true_debug(hrs::is_power_of_two(alignment), "Alignment is not power of two!");

		ResourceType res_type =
			(info.tiling == vk::ImageTiling::eLinear ? ResourceType::Linear : ResourceType::NonLinear);
		auto [u_image_res, u_image] = device.createImageUnique(info);
		if(u_image_res != vk::Result::eSuccess)
			return {u_image_res};

		hrs::error common_err = AllocatorResult::NoSatisfiedMemoryTypes;
		auto req = device.getImageMemoryRequirements(u_image.get());
		if(req.alignment < alignment)
			req.alignment = alignment;

		for(auto mem_type_it = memory_types.begin(); mem_type_it != memory_types.end(); mem_type_it++)
		{
			if(!is_memory_type_satisfy(*mem_type_it, op, desired_props, req))
				continue;

			auto block_pool_bind_exp = mem_type_it->Bind(u_image.get(), res_type, req, flags, calc);
			if(block_pool_bind_exp)
				return BoundedImage(std::move(block_pool_bind_exp.value()), mem_type_it, res_type, u_image.release());
			else if(is_non_terminate_error(block_pool_bind_exp.error()))
				common_err = block_pool_bind_exp.error();
			else
				return {block_pool_bind_exp.error()};
		}

		return common_err;
	}

	void Allocator::Release(const BoundedBuffer &bounded_buffer, MemoryPoolOnEmptyPolicy policy)
	{
		if(!bounded_buffer.IsCreated())
			return;

		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(memory_types, bounded_buffer.memory_type),
							   "Memory type that tied up with buffer is not part of this allocator");

		bounded_buffer.memory_type->Release(bounded_buffer.buffer,
											bounded_buffer.block_bind_pool,
											policy);
	}

	void Allocator::Release(const BoundedImage &bounded_image, MemoryPoolOnEmptyPolicy policy)
	{
		if(!bounded_image.IsCreated())
			return;

		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(memory_types, bounded_image.memory_type),
							   "Memory type that tied up with buffer is not part of this allocator");

		bounded_image.memory_type->Release(bounded_image.image,
										   bounded_image.res_type,
										   bounded_image.block_bind_pool,
										   policy);
	}

	bool Allocator::is_memory_type_satisfy(const MemoryType &mem_type,
										   MemoryTypeSatisfyOp op,
										   vk::MemoryPropertyFlags desired_props,
										   const vk::MemoryRequirements &req) const noexcept
	{
		if(!mem_type.IsSatisfy(req))
			return false;

		if(op == MemoryTypeSatisfyOp::Any)
			return mem_type.IsSatisfyAny(desired_props);

		return mem_type.IsSatisfyOnly(desired_props);
	}

	bool Allocator::is_non_terminate_error(const hrs::error &err) const noexcept
	{
		if(err.holds<MemoryPoolResult>())
			return true;

		vk::Result res = err.revive<vk::Result>();
		switch(res)
		{
			case vk::Result::eErrorOutOfDeviceMemory:
			case vk::Result::eErrorOutOfHostMemory:
			case vk::Result::eErrorMemoryMapFailed:
				return true;
				break;
			default:
				return false;
				break;
		}
	}
};
