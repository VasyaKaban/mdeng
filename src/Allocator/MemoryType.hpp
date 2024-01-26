#pragma once

#include "AllocatorTypes.hpp"
#include "../hrs/flags.hpp"

namespace FireLand
{
	enum class MemoryTypeSatisfyOperation
	{
		Any,
		Only
	};

	class MemoryType
	{
	public:
		MemoryType(vk::MemoryType _mem_type = {}, std::uint32_t _index = {});
		~MemoryType() = default;
		MemoryType(const MemoryType &) = delete;
		MemoryType(MemoryType &&mtype) noexcept;
		MemoryType & operator=(const MemoryType &) = delete;
		MemoryType & operator=(MemoryType &&) = delete;

		constexpr vk::MemoryType GetMemoryType() const noexcept;
		constexpr std::uint32_t GetIndexMask() const noexcept;
		constexpr std::uint32_t GetIndex() const noexcept;
		constexpr bool IsMappable() const noexcept;
		constexpr bool IsSatisfy(const vk::MemoryRequirements &req) const noexcept;
		constexpr bool IsSatisfy(vk::MemoryPropertyFlags desired, MemoryTypeSatisfyOperation op) const noexcept;
		constexpr bool IsSatisfyAny(vk::MemoryPropertyFlags desired) const noexcept;
		constexpr bool IsSatisfyOnly(vk::MemoryPropertyFlags desired) const noexcept;

		constexpr std::list<MemoryPool> & GetPools() noexcept;
		constexpr const std::list<MemoryPool> & GetPools() const noexcept;

		hrs::expected<std::list<MemoryPool>::iterator, vk::Result>
		AllocatePool(vk::Device device,
					 vk::DeviceSize size,
					 MemoryPoolResourceType resource_type,
					 bool map_memory);

		hrs::expected<BoundedBuffer, AllocationError>
		BindToExistedPool(vk::Device device,
						  const vk::MemoryRequirements &req,
						  vk::Buffer &buffer,
						  hrs::flags<AllocationFlags> flags,
						  vk::DeviceSize buffer_image_granularity);

		hrs::expected<BoundedBuffer, AllocationError>
		BindToSeparatePool(vk::Device device,
						   vk::MemoryRequirements req,
						   vk::DeviceSize pool_size,
						   vk::Buffer &buffer,
						   hrs::flags<AllocationFlags> flags,
						   vk::DeviceSize buffer_image_granularity);

		hrs::expected<BoundedImage, AllocationError>
		BindToExistedPool(vk::Device device,
						  const vk::MemoryRequirements &req,
						  vk::Image &image,
						  MemoryPoolResourceType resource_type,
						  hrs::flags<AllocationFlags> flags,
						  vk::DeviceSize buffer_image_granularity);

		hrs::expected<BoundedImage, AllocationError>
		BindToSeparatePool(vk::Device device,
						   vk::MemoryRequirements req,
						   vk::DeviceSize pool_size,
						   vk::Image &image,
						   MemoryPoolResourceType resource_type,
						   hrs::flags<AllocationFlags> flags,
						   vk::DeviceSize buffer_image_granularity);


		void Destroy(vk::Device device) noexcept;

		void DestroyEmptyPools(vk::Device device) noexcept;

		void DestroyPool(vk::Device device, std::list<MemoryPool>::iterator handle) noexcept;

	private:

		vk::MemoryType mem_type;
		std::uint32_t index;
		std::list<MemoryPool> pools;
	};

	inline MemoryType::MemoryType(vk::MemoryType _mem_type, std::uint32_t _index)
		: mem_type(_mem_type), index(_index) {}

	inline MemoryType::MemoryType(MemoryType &&mtype) noexcept
		: mem_type(mtype.mem_type), index(mtype.index), pools(std::move(mtype.pools)) {}

	constexpr vk::MemoryType MemoryType::GetMemoryType() const noexcept
	{
		return mem_type;
	}

	constexpr std::uint32_t MemoryType::GetIndexMask() const noexcept
	{
		return 1 << index;
	}

	constexpr std::uint32_t MemoryType::GetIndex() const noexcept
	{
		return index;
	}

	constexpr bool MemoryType::IsMappable() const noexcept
	{
		return static_cast<bool>(mem_type.propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible);
	}

	constexpr bool MemoryType::IsSatisfy(const vk::MemoryRequirements &req) const noexcept
	{
		return req.memoryTypeBits & GetIndexMask();
	}

	constexpr bool MemoryType::IsSatisfy(vk::MemoryPropertyFlags desired,
										 MemoryTypeSatisfyOperation op) const noexcept
	{
		return (op == MemoryTypeSatisfyOperation::Any ? IsSatisfyAny(desired) : IsSatisfyOnly(desired));
	}

	constexpr bool MemoryType::IsSatisfyAny(vk::MemoryPropertyFlags desired) const noexcept
	{
		return static_cast<bool>((mem_type.propertyFlags & desired));
	}

	constexpr bool MemoryType::IsSatisfyOnly(vk::MemoryPropertyFlags desired) const noexcept
	{
		return mem_type.propertyFlags == desired;
	}

	constexpr std::list<MemoryPool> & MemoryType::GetPools() noexcept
	{
		return pools;
	}

	constexpr const std::list<MemoryPool> & MemoryType::GetPools() const noexcept
	{
		return pools;
	}

	inline hrs::expected<std::list<MemoryPool>::iterator, vk::Result>
	MemoryType::AllocatePool(vk::Device device,
							 vk::DeviceSize size,
							 MemoryPoolResourceType resource_type,
							 bool map_memory)
	{
		hrs::assert_true_debug((map_memory ? map_memory == IsMappable() : true), "Memory cannot be mapped!");
		hrs::assert_true_debug(device, "Device isn't created yet!");

		vk::MemoryAllocateInfo info(size, index);
		auto u_memory = device.allocateMemoryUnique(info);
		if(u_memory.result != vk::Result::eSuccess)
			return u_memory.result;

		std::byte *map_ptr = nullptr;
		if(map_memory)
		{
			auto map_res = device.mapMemory(u_memory.value.get(), 0, size);
			if(map_res.result != vk::Result::eSuccess)
				return map_res.result;

			map_ptr = static_cast<std::byte *>(map_res.value);
		}

		pools.push_back(MemoryPool(Memory(u_memory.value.release(), size, map_ptr), resource_type));
		return std::prev(pools.end());
	}

	inline hrs::expected<BoundedBuffer, AllocationError>
	MemoryType::BindToExistedPool(vk::Device device,
								  const vk::MemoryRequirements &req,
								  vk::Buffer &buffer,
								  hrs::flags<AllocationFlags> flags,
								  vk::DeviceSize buffer_image_granularity)
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		hrs::assert_true_debug(buffer, "Buffer isn't created yet!");

		if(!(req.memoryTypeBits & GetIndexMask()))
			return {AllocationResult::NoSatisfiedMemoryTypes};

		bool is_pool_not_enough_space = false;
		for(auto pool_it = pools.begin(); pool_it != pools.end(); pool_it++)
		{
			bool can_be_placed = (pool_it->GetResourceType() == MemoryPoolResourceType::Linear ?
									  true :
									  static_cast<bool>(flags & AllocationFlags::AllowCreationWithMixedResources));
			if(can_be_placed)
			{
				auto bind_res = pool_it->Bind(req, device, buffer, buffer_image_granularity);
				if(!bind_res)
					return {bind_res.error()};

				if(bind_res.value())
					return BoundedBuffer(buffer, GetIndex(), pool_it, bind_res.value().value());
				else
					is_pool_not_enough_space = true;
			}
		}

		if(is_pool_not_enough_space)
			return {AllocationResult::NotEnoughSpaceInPool};

		return {AllocationResult::NoSatisfiedMemoryPools};
	}

	inline hrs::expected<BoundedBuffer, AllocationError>
	MemoryType::BindToSeparatePool(vk::Device device,
								   vk::MemoryRequirements req,
								   vk::DeviceSize pool_size,
								   vk::Buffer &buffer,
								   hrs::flags<AllocationFlags> flags,
								   vk::DeviceSize buffer_image_granularity)
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		hrs::assert_true_debug(buffer, "Buffer isn't created yet!");
		hrs::assert_true_debug(req.size <= pool_size,
							   "Required size={} is bigger than pool size={}!",
							   req.size, pool_size);

		if(!(req.memoryTypeBits & GetIndexMask()))
			return {AllocationResult::NoSatisfiedMemoryTypes};

		auto pool_exp = AllocatePool(device,
									 pool_size,
									 (flags & AllocationFlags::AllocateWithMixedResources ?
										  MemoryPoolResourceType::Mixed :
										  MemoryPoolResourceType::Linear),
									 static_cast<bool>(flags & AllocationFlags::MapAllocatedMemory));

		if(!pool_exp)
			return AllocationError(pool_exp.error());

		auto pool_handle = pool_exp.value();

		if(flags & AllocationFlags::BindToWholeMemory)
			req.size = pool_handle->GetSize();

		auto bind_res = pool_handle->Bind(req, device, buffer, buffer_image_granularity);
		if(!bind_res)
		{
			DestroyPool(device, pool_handle);
			return AllocationError(bind_res.error());
		}

		if(!bind_res.value())
		{
			DestroyPool(device, pool_handle);
			return AllocationError(AllocationResult::NotEnoughSpaceInPool);
		}

		return BoundedBuffer(buffer, GetIndex(), pool_exp.value(), bind_res.value().value());
	}

	inline hrs::expected<BoundedImage, AllocationError>
	MemoryType::BindToExistedPool(vk::Device device,
								  const vk::MemoryRequirements &req,
								  vk::Image &image,
								  MemoryPoolResourceType resource_type,
								  hrs::flags<AllocationFlags> flags,
								  vk::DeviceSize buffer_image_granularity)
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		hrs::assert_true_debug(image, "Image isn't created yet!");

		if(!(req.memoryTypeBits & GetIndexMask()))
			return {AllocationResult::NoSatisfiedMemoryTypes};

		hrs::assert_true_debug(resource_type == MemoryPoolResourceType::Linear ||
								   resource_type == MemoryPoolResourceType::NonLinear,
							   "Bad resource type!");

		bool is_pool_not_enough_space = false;
		for(auto pool_it = pools.begin(); pool_it != pools.end(); pool_it++)
		{
			bool can_be_placed = (pool_it->GetResourceType() == resource_type ?
									  true :
									  static_cast<bool>(flags & AllocationFlags::AllowCreationWithMixedResources));
			if(can_be_placed)
			{
				auto bind_res = pool_it->Bind(req, device, image, resource_type, buffer_image_granularity);
				if(!bind_res)
					return {bind_res.error()};

				if(bind_res.value())
					return BoundedImage(image, GetIndex(), pool_it, bind_res.value().value());
				else
					is_pool_not_enough_space = true;
			}
		}

		if(is_pool_not_enough_space)
			return {AllocationResult::NotEnoughSpaceInPool};

		return {AllocationResult::NoSatisfiedMemoryPools};
	}

	inline hrs::expected<BoundedImage, AllocationError>
	MemoryType::BindToSeparatePool(vk::Device device,
								   vk::MemoryRequirements req,
								   vk::DeviceSize pool_size,
								   vk::Image &image,
								   MemoryPoolResourceType resource_type,
								   hrs::flags<AllocationFlags> flags,
								   vk::DeviceSize buffer_image_granularity)
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		hrs::assert_true_debug(image, "Image isn't created yet!");
		hrs::assert_true_debug(req.size <= pool_size,
							   "Required size={} is bigger than pool size={}!",
							   req.size, pool_size);

		if(!(req.memoryTypeBits & GetIndexMask()))
			return {AllocationResult::NoSatisfiedMemoryTypes};

		hrs::assert_true_debug(resource_type == MemoryPoolResourceType::Linear ||
								   resource_type == MemoryPoolResourceType::NonLinear,
							   "Bad resource type!");

		auto pool_exp = AllocatePool(device,
									 pool_size,
									 (flags & AllocationFlags::AllocateWithMixedResources ?
										  MemoryPoolResourceType::Mixed :
										  resource_type),
									 static_cast<bool>(flags & AllocationFlags::MapAllocatedMemory));

		if(!pool_exp)
			return AllocationError(pool_exp.error());

		auto pool_handle = pool_exp.value();

		if(flags & AllocationFlags::BindToWholeMemory)
			req.size = pool_handle->GetSize();

		auto bind_res = pool_handle->Bind(req, device, image, resource_type, buffer_image_granularity);
		if(!bind_res)
		{
			DestroyPool(device, pool_handle);
			return AllocationError(bind_res.error());
		}

		if(!bind_res.value())
		{
			DestroyPool(device, pool_handle);
			return AllocationError(AllocationResult::NotEnoughSpaceInPool);
		}

		return BoundedImage(image, GetIndex(), pool_exp.value(), bind_res.value().value());
	}

	inline void MemoryType::Destroy(vk::Device device) noexcept
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		for(auto &pool : pools)
			pool.Destroy(device);

		pools.clear();
	}

	inline void MemoryType::DestroyEmptyPools(vk::Device device) noexcept
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");

		std::erase_if(pools, [](const MemoryPool &pool) noexcept
		{
			return pool.IsEmpty();
		});
	}

	inline void MemoryType::DestroyPool(vk::Device device, std::list<MemoryPool>::iterator handle) noexcept
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(pools, handle),
							   "Handle is not a part of this object!");

		hrs::assert_true_debug(device, "Device isn't created yet!");

		handle->Destroy(device);
		pools.erase(handle);
	}
};
