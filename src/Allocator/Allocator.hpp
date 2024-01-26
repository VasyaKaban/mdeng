#pragma once

#include "MemoryType.hpp"
#include "../hrs/flags.hpp"
#include "../hrs/expected.hpp"

namespace FireLand
{
	class Allocator
	{
	public:

		using SizeCalculateFunctionType = vk::DeviceSize (*)(const MemoryType &,
															 const vk::MemoryHeap &,
															 vk::DeviceSize,
															 vk::DeviceSize) noexcept;

		constexpr static vk::DeviceSize default_size_calculate(const MemoryType &memory_type,
															   const vk::MemoryHeap &heap,
															   vk::DeviceSize requested_size,
															   vk::DeviceSize previous_call_size) noexcept;

		Allocator(vk::Device _device = {},
				  vk::PhysicalDevice _ph_device = {},
				  SizeCalculateFunctionType _size_calculate_function_handle = default_size_calculate);
		~Allocator();
		Allocator(const Allocator &) = delete;
		Allocator(Allocator &&allocator) noexcept;
		Allocator & operator=(const Allocator &) = delete;
		Allocator & operator=(Allocator &&allocator) noexcept;

		constexpr const std::vector<vk::MemoryHeap> & GetHeaps() const noexcept;
		constexpr const std::vector<MemoryType> & GetMemoryTypes() const noexcept;
		constexpr vk::Device GetDevice() const noexcept;
		constexpr vk::DeviceSize GetBufferImageGranularity() const noexcept;
		constexpr SizeCalculateFunctionType GetSizeCalculateFunctionHandle() const noexcept;

		constexpr SizeCalculateFunctionType
		SetSizeCalculateFunctionHandle(SizeCalculateFunctionType func) noexcept;

		constexpr bool IsCreated() const noexcept;

		constexpr bool IsGranularityFree() const noexcept;

		hrs::expected<BoundedBuffer, AllocationError>
		Create(const vk::BufferCreateInfo &info,
			   vk::MemoryPropertyFlags desired,
			   hrs::flags<AllocationFlags> allocation_flags,
			   MemoryTypeSatisfyOperation op);

		hrs::expected<BoundedImage, AllocationError>
		Create(const vk::ImageCreateInfo &info,
			   vk::MemoryPropertyFlags desired,
			   hrs::flags<AllocationFlags> allocation_flags,
			   MemoryTypeSatisfyOperation op);


		void Destroy(BoundedBuffer &bounded_buffer, bool destroy_pool_on_empty);
		void Destroy(BoundedBuffer &&bounded_buffer, bool destroy_pool_on_empty);

		void Destroy(BoundedImage &bounded_image, bool destroy_pool_on_empty);
		void Destroy(BoundedImage &&bounded_image, bool destroy_pool_on_empty);

		constexpr MemoryType & GetMemoryType(std::size_t index) noexcept;
		constexpr const MemoryType & GetMemoryType(std::size_t index) const noexcept;

	private:

		hrs::expected<BoundedBuffer, AllocationError>
		try_allocate_separate_and_bind(vk::Buffer &buffer,
									   const vk::MemoryRequirements &req,
									   hrs::flags<AllocationFlags> allocation_flags,
									   std::uint32_t memory_types_mask);

		hrs::expected<BoundedBuffer, AllocationError>
		try_bind_to_existed(vk::Buffer &buffer,
							const vk::MemoryRequirements &req,
							hrs::flags<AllocationFlags> allocation_flags,
							vk::MemoryPropertyFlags desired,
							std::uint32_t &memory_types_mask,
							MemoryTypeSatisfyOperation op);

		hrs::expected<BoundedImage, AllocationError>
		try_allocate_separate_and_bind(vk::Image &image,
									   const vk::MemoryRequirements &req,
									   hrs::flags<AllocationFlags> allocation_flags,
									   MemoryPoolResourceType resource_type,
									   std::uint32_t memory_types_mask);

		hrs::expected<BoundedImage, AllocationError>
		try_bind_to_existed(vk::Image &image,
							const vk::MemoryRequirements &req,
							hrs::flags<AllocationFlags> allocation_flags,
							vk::MemoryPropertyFlags desired,
							MemoryPoolResourceType resource_type,
							std::uint32_t &memory_types_mask,
							MemoryTypeSatisfyOperation op);

		vk::Device device;
		std::vector<MemoryType> memory_types;
		std::vector<vk::MemoryHeap> heaps;
		vk::DeviceSize buffer_image_granularity;
		SizeCalculateFunctionType size_calculate_function_handle;
	};

	constexpr vk::DeviceSize Allocator::default_size_calculate(const MemoryType &memory_type,
															   const vk::MemoryHeap &heap,
															   vk::DeviceSize requested_size,
															   vk::DeviceSize previous_call_size) noexcept
	{
		constexpr vk::DeviceSize delim = 32;
		if(previous_call_size == 0)//no previous call
		{
			vk::DeviceSize size_power = heap.size / delim;
			return hrs::round_up_size_to_alignment(requested_size, size_power);
		}
		else//not enough space for size returned from previous call
		{
			vk::DeviceSize new_size = previous_call_size >> 1;
			if(new_size > requested_size)
				return requested_size;

			return new_size;
		}
	}

	inline Allocator::Allocator(vk::Device _device,
								vk::PhysicalDevice _ph_device,
								SizeCalculateFunctionType _size_calculate_function_handle)
	{
		hrs::assert_true_debug(_device, "Device isn't created yet!");
		hrs::assert_true_debug(_ph_device, "Physical device isn't created yet!");
		hrs::assert_true_debug(_size_calculate_function_handle, "Size calculate function references to null!");

		auto mem_props = _ph_device.getMemoryProperties();
		heaps.reserve(mem_props.memoryHeapCount);
		memory_types.reserve(mem_props.memoryTypeCount);
		for(std::size_t i = 0 ; i < mem_props.memoryHeapCount; i++)
			heaps.push_back(mem_props.memoryHeaps[i]);

		for(std::size_t i = 0 ; i < mem_props.memoryTypeCount; i++)
			memory_types.emplace_back(mem_props.memoryTypes[i], i);

		device = _device;
		buffer_image_granularity = _ph_device.getProperties().limits.bufferImageGranularity;
		size_calculate_function_handle = _size_calculate_function_handle;
	}

	inline Allocator::~Allocator()
	{
		if(IsCreated())
		{
			for(auto &mem_type : memory_types)
				mem_type.Destroy(device);

			device = VK_NULL_HANDLE;
			memory_types.clear();
			heaps.clear();
		}
	}

	inline Allocator::Allocator(Allocator &&allocator) noexcept
		: device(allocator.device),
		  buffer_image_granularity(allocator.buffer_image_granularity),
		  size_calculate_function_handle(allocator.size_calculate_function_handle),
		  memory_types(std::move(allocator.memory_types)),
		  heaps(std::move(allocator.heaps))
	{
		allocator.device = VK_NULL_HANDLE;
	}

	inline Allocator & Allocator::operator=(Allocator &&allocator) noexcept
	{
		this->~Allocator();
		device = allocator.device;
		buffer_image_granularity = allocator.buffer_image_granularity;
		size_calculate_function_handle = allocator.size_calculate_function_handle;
		memory_types = std::move(allocator.memory_types);
		heaps = std::move(allocator.heaps);
		allocator.device = VK_NULL_HANDLE;

		return *this;
	}

	constexpr const std::vector<vk::MemoryHeap> & Allocator::GetHeaps() const noexcept
	{
		return heaps;
	}

	constexpr const std::vector<MemoryType> & Allocator::GetMemoryTypes() const noexcept
	{
		return memory_types;
	}

	constexpr vk::Device Allocator::GetDevice() const noexcept
	{
		return device;
	}

	constexpr vk::DeviceSize Allocator::GetBufferImageGranularity() const noexcept
	{
		return buffer_image_granularity;
	}

	constexpr Allocator::SizeCalculateFunctionType Allocator::GetSizeCalculateFunctionHandle() const noexcept
	{
		return size_calculate_function_handle;
	}

	constexpr Allocator::SizeCalculateFunctionType
	Allocator::SetSizeCalculateFunctionHandle(SizeCalculateFunctionType func) noexcept
	{
		hrs::assert_true_debug(func, "Size calculation function references to null!");
		return std::exchange(size_calculate_function_handle, func);
	}

	constexpr bool Allocator::IsCreated() const noexcept
	{
		return device;
	}

	constexpr bool Allocator::IsGranularityFree() const noexcept
	{
		return buffer_image_granularity == 1;
	}

	inline hrs::expected<BoundedBuffer, AllocationError>
	Allocator::Create(const vk::BufferCreateInfo &info,
					  vk::MemoryPropertyFlags desired,
					  hrs::flags<AllocationFlags> allocation_flags,
					  MemoryTypeSatisfyOperation op)
	{
		hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");
		hrs::assert_true_debug((allocation_flags & AllocationFlags::AllocateSeparateMemory ?
									!(allocation_flags & AllocationFlags::CreateOnlyOnExistedPools) :
									true),
							   "Conflicting flags AllocateSeparateMemory and CreateOnlyOnExistedPools are set!");

		auto buffer_res = device.createBuffer(info);
		if(buffer_res.result != vk::Result::eSuccess)
			return {buffer_res.result};

		std::uint32_t satisfied_memory_types_mask = 0;
		auto req = device.getBufferMemoryRequirements(buffer_res.value);

		hrs::expected<BoundedBuffer, AllocationError> bind_res;
		if((allocation_flags & AllocationFlags::CreateOnlyOnExistedPools) ||
			!(allocation_flags & AllocationFlags::AllocateSeparateMemory))
		{
			bind_res = try_bind_to_existed(buffer_res.value,
										   req,
										   allocation_flags,
										   desired,
										   satisfied_memory_types_mask,
										   op);

			if(bind_res)
				return bind_res;
			else if(/*!bind_res &&*/ bind_res.error().keeps<vk::Result>()/*bind_res.error().IsBadVulkanResult()*/)
			{
				device.destroy(buffer_res.value);
				return bind_res;
			}
		}

		if((allocation_flags & AllocationFlags::AllocateSeparateMemory) ||
			!(allocation_flags & AllocationFlags::CreateOnlyOnExistedPools))
		{
			bind_res = try_allocate_separate_and_bind(buffer_res.value,
													  req,
													  allocation_flags,
													  (1 << memory_types.size()) - 1);
		}

		if(!bind_res)
			device.destroy(buffer_res.value);

		return bind_res;
	}

	inline hrs::expected<BoundedImage, AllocationError>
	Allocator::Create(const vk::ImageCreateInfo &info,
					  vk::MemoryPropertyFlags desired,
					  hrs::flags<AllocationFlags> allocation_flags,
					  MemoryTypeSatisfyOperation op)
	{
		hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");
		hrs::assert_true_debug((allocation_flags & AllocationFlags::AllocateSeparateMemory ?
									!(allocation_flags & AllocationFlags::CreateOnlyOnExistedPools) :
									true),
							   "Conflicting flags AllocateSeparateMemory and CreateOnlyOnExistedPools are set!");

		auto image_res = device.createImage(info);
		if(image_res.result != vk::Result::eSuccess)
			return {image_res.result};

		MemoryPoolResourceType resource_type = (info.tiling == vk::ImageTiling::eLinear ?
													MemoryPoolResourceType::Linear :
													MemoryPoolResourceType::NonLinear);

		std::uint32_t satisfied_memory_types_mask = 0;
		auto req = device.getImageMemoryRequirements(image_res.value);

		hrs::expected<BoundedImage, AllocationError> bind_res;
		if((allocation_flags & AllocationFlags::CreateOnlyOnExistedPools) ||
			!(allocation_flags & AllocationFlags::AllocateSeparateMemory))
		{
			bind_res = try_bind_to_existed(image_res.value,
										   req,
										   allocation_flags,
										   desired,
										   resource_type,
										   satisfied_memory_types_mask,
										   op);

			if(bind_res)
				return bind_res;
			else if(/*!bind_res &&*/ bind_res.error().keeps<vk::Result>()/*bind_res.error().IsBadVulkanResult()*/)
			{
				device.destroy(image_res.value);
				return bind_res;
			}
		}

		if((allocation_flags & AllocationFlags::AllocateSeparateMemory) ||
			!(allocation_flags & AllocationFlags::CreateOnlyOnExistedPools))
		{
			bind_res = try_allocate_separate_and_bind(image_res.value,
													  req,
													  allocation_flags,
													  resource_type,
													  (1 << memory_types.size()) - 1);
		}

		if(!bind_res)
			device.destroy(image_res.value);

		return bind_res;
	}

	inline void Allocator::Destroy(BoundedBuffer &bounded_buffer, bool destroy_pool_on_empty)
	{
		hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");

		if(!bounded_buffer.buffer)
			return;

		hrs::assert_true_debug(bounded_buffer.memory_type_index < memory_types.size(),
							   "Memory type index={} is out of acceptable range=({}, {})!",
							   bounded_buffer.memory_type_index, 0, memory_types.size() - 1);


		auto &memory_type = memory_types[bounded_buffer.memory_type_index];
		auto &pools = memory_type.GetPools();
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(pools, bounded_buffer.memory_pool_handle),
							   "Pool handle is not a part of this memory type!");


		bounded_buffer.memory_pool_handle->Release(bounded_buffer.block);
		device.destroy(bounded_buffer.buffer);
		if(bounded_buffer.memory_pool_handle->IsEmpty() && destroy_pool_on_empty)
			memory_type.DestroyPool(device, bounded_buffer.memory_pool_handle);

		bounded_buffer.buffer = VK_NULL_HANDLE;
	}

	inline void Allocator::Destroy(BoundedBuffer &&bounded_buffer, bool destroy_pool_on_empty)
	{
		Destroy(bounded_buffer, destroy_pool_on_empty);
	}

	inline void Allocator::Destroy(BoundedImage &bounded_image, bool destroy_pool_on_empty)
	{
		hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");

		if(!bounded_image.image)
			return;

		hrs::assert_true_debug(bounded_image.memory_type_index < memory_types.size(),
							   "Memory type index={} is out of acceptable range=({}, {})!",
							   bounded_image.memory_type_index, 0, memory_types.size() - 1);


		auto &memory_type = memory_types[bounded_image.memory_type_index];
		auto &pools = memory_type.GetPools();
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(pools, bounded_image.memory_pool_handle),
							   "Pool handle is not a part of this memory type!");


		bounded_image.memory_pool_handle->Release(bounded_image.block);
		device.destroy(bounded_image.image);
		if(bounded_image.memory_pool_handle->IsEmpty() && destroy_pool_on_empty)
			memory_type.DestroyPool(device, bounded_image.memory_pool_handle);

		bounded_image.image = VK_NULL_HANDLE;
	}

	inline void Allocator::Destroy(BoundedImage &&bounded_image, bool destroy_pool_on_empty)
	{
		Destroy(bounded_image, destroy_pool_on_empty);
	}

	constexpr MemoryType & Allocator::GetMemoryType(std::size_t index) noexcept
	{
		return memory_types[index];
	}

	constexpr const MemoryType & Allocator::GetMemoryType(std::size_t index) const noexcept
	{
		return memory_types[index];
	}

	inline hrs::expected<BoundedBuffer, AllocationError>
	Allocator::try_allocate_separate_and_bind(vk::Buffer &buffer,
											  const vk::MemoryRequirements &req,
											  hrs::flags<AllocationFlags> allocation_flags,
											  std::uint32_t memory_types_mask)
	{
		if(memory_types_mask == 0)
			return {AllocationResult::NoSatisfiedMemoryTypes};

		std::uint32_t index = 0;
		vk::Result oom_result = vk::Result::eSuccess;
		while(memory_types_mask != 0)
		{
			if(memory_types_mask & 0x1)
			{
				MemoryType &mem_type = memory_types[index];

				vk::DeviceSize previous_size = 0;
				while(previous_size != req.size)
				{
					previous_size = size_calculate_function_handle(mem_type,
																   heaps[mem_type.GetMemoryType().heapIndex],
																   req.size,
																   previous_size);

					hrs::assert_true_debug(previous_size >= req.size,
										   "Newly calculated size={} must be bigger or equal than requires size={}!",
										   previous_size, req.size);

					auto bind_res = mem_type.BindToSeparatePool(device,
																req,
																previous_size,
																buffer,
																allocation_flags,
																buffer_image_granularity);

					if(bind_res)
						return bind_res;
					else if(/*bind_res.error().IsBadVulkanResult()*/bind_res.error().keeps<vk::Result>())
					{
						vk::Result res = bind_res.error().get<vk::Result>();
						if(res == vk::Result::eErrorOutOfHostMemory ||
							res == vk::Result::eErrorOutOfDeviceMemory)
							oom_result = res;
						else
							return bind_res;
					}
				}
			}

			index++;
			memory_types_mask >>= 1;
		}

		if(oom_result != vk::Result::eSuccess)
			return {oom_result};

		return {AllocationResult::NoSatisfiedMemoryTypes};
	}

	inline hrs::expected<BoundedBuffer, AllocationError>
	Allocator::try_bind_to_existed(vk::Buffer &buffer,
								   const vk::MemoryRequirements &req,
								   hrs::flags<AllocationFlags> allocation_flags,
								   vk::MemoryPropertyFlags desired,
								   std::uint32_t &memory_types_mask,
								   MemoryTypeSatisfyOperation op)
	{
		bool is_no_space_in_pools = false;
		for(auto &mem_type : memory_types)
		{
			if(mem_type.IsSatisfy(req) &&
				(op == MemoryTypeSatisfyOperation::Any ?
												mem_type.IsSatisfyAny(desired) :
												mem_type.IsSatisfyOnly(desired)))
			{
				memory_types_mask |= mem_type.GetIndexMask();
				auto bind_res = mem_type.BindToExistedPool(device,
														   req,
														   buffer,
														   allocation_flags,
														   buffer_image_granularity);

				if(bind_res)
					return bind_res;
				else if(/*bind_res.error().IsBadVulkanResult()*/bind_res.error().keeps<vk::Result>())
					return bind_res;
				else if(/*bind_res.error().alloc_result*/bind_res.error().get<AllocationResult>() == AllocationResult::NotEnoughSpaceInPool)
					is_no_space_in_pools = true;
			}
		}

		if(memory_types_mask == 0)
			return {AllocationResult::NoSatisfiedMemoryTypes};
		else if(is_no_space_in_pools)
			return {AllocationResult::NotEnoughSpaceInPool};
		else
			return {AllocationResult::NoSatisfiedMemoryPools};
	}

	inline hrs::expected<BoundedImage, AllocationError>
	Allocator::try_allocate_separate_and_bind(vk::Image &image,
											  const vk::MemoryRequirements &req,
											  hrs::flags<AllocationFlags> allocation_flags,
											  MemoryPoolResourceType resource_type,
											  std::uint32_t memory_types_mask)
	{
		if(memory_types_mask == 0)
			return {AllocationResult::NoSatisfiedMemoryTypes};

		std::uint32_t index = 0;
		vk::Result oom_result = vk::Result::eSuccess;
		while(memory_types_mask != 0)
		{
			if(memory_types_mask & 0x1)
			{
				MemoryType &mem_type = memory_types[index];

				vk::DeviceSize previous_size = 0;
				while(previous_size != req.size)
				{
					previous_size = size_calculate_function_handle(mem_type,
																   heaps[mem_type.GetMemoryType().heapIndex],
																   req.size,
																   previous_size);

					hrs::assert_true_debug(previous_size >= req.size,
										   "Newly calculated size={} must be bigger or equal than requires size={}!",
										   previous_size, req.size);

					auto bind_res = mem_type.BindToSeparatePool(device,
																req.size,
																previous_size,
																image,
																resource_type,
																allocation_flags,
																buffer_image_granularity);

					if(bind_res)
						return bind_res;
					else if(/*bind_res.error().IsBadVulkanResult()*/bind_res.error().keeps<vk::Result>())
					{
						vk::Result res = bind_res.error().get<vk::Result>();
						if(res == vk::Result::eErrorOutOfHostMemory ||
							res == vk::Result::eErrorOutOfDeviceMemory)
							oom_result = res;
						else
							return bind_res;
					}
				}
			}

			index++;
			memory_types_mask >>= 1;
		}

		if(oom_result != vk::Result::eSuccess)
			return {oom_result};

		return {AllocationResult::NoSatisfiedMemoryTypes};
	}

	inline hrs::expected<BoundedImage, AllocationError>
	Allocator::try_bind_to_existed(vk::Image &image,
								   const vk::MemoryRequirements &req,
								   hrs::flags<AllocationFlags> allocation_flags,
								   vk::MemoryPropertyFlags desired,
								   MemoryPoolResourceType resource_type,
								   std::uint32_t &memory_types_mask,
								   MemoryTypeSatisfyOperation op)
	{
		bool is_no_space_in_pools = false;
		for(auto &mem_type : memory_types)
		{
			if(mem_type.IsSatisfy(req) &&
				(op == MemoryTypeSatisfyOperation::Any ?
					 mem_type.IsSatisfyAny(desired) :
					 mem_type.IsSatisfyOnly(desired)))
			{
				memory_types_mask |= mem_type.GetIndexMask();
				auto bind_res = mem_type.BindToExistedPool(device,
														   req,
														   image,
														   resource_type,
														   allocation_flags,
														   buffer_image_granularity);

				if(bind_res)
					return bind_res;
				else if(/*bind_res.error().IsBadVulkanResult()*/bind_res.error().keeps<vk::Result>())
					return bind_res;
				else if(/*bind_res.error().alloc_result*/bind_res.error().get<AllocationResult>() == AllocationResult::NotEnoughSpaceInPool)
					is_no_space_in_pools = true;
			}
		}

		if(memory_types_mask == 0)
			return {AllocationResult::NoSatisfiedMemoryTypes};
		else if(is_no_space_in_pools)
			return {AllocationResult::NotEnoughSpaceInPool};
		else
			return {AllocationResult::NoSatisfiedMemoryPools};
	}
};
