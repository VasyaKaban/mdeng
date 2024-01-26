#pragma once

#include "Memory.hpp"
#include "../hrs/relaxed_free_blocks_chain.hpp"
#include "../hrs/expected.hpp"

namespace FireLand
{
	enum class MemoryPoolResourceType
	{
		None,
		Linear,
		NonLinear,
		Mixed
	};

	class MemoryPool
	{
	public:
		MemoryPool(Memory &&_memory, MemoryPoolResourceType _resource_type = MemoryPoolResourceType::None);
		~MemoryPool() = default;
		MemoryPool(const MemoryPool &) = delete;
		MemoryPool(MemoryPool &&pool) noexcept;
		MemoryPool & operator=(const MemoryPool &) = delete;
		MemoryPool & operator=(MemoryPool &&pool) = delete;

		constexpr vk::DeviceSize GetSize() const noexcept;
		constexpr MemoryPoolResourceType GetResourceType() const noexcept;

		constexpr Memory GetMemory() noexcept;
		constexpr const Memory GetMemory() const noexcept;
		constexpr bool IsEmpty() const noexcept;

		vk::Result MapMemory(vk::Device device) noexcept;

		hrs::expected<std::optional<hrs::block<vk::DeviceSize>>, vk::Result>
		Bind(const vk::MemoryRequirements &req,
			 vk::Device device,
			 vk::Buffer &buffer,
			 vk::DeviceSize buffer_image_granularity);

		hrs::expected<std::optional<hrs::block<vk::DeviceSize>>, vk::Result>
		Bind(const vk::MemoryRequirements &req,
			 vk::Device device,
			 vk::Image &image,
			 MemoryPoolResourceType req_resource_type,
			 vk::DeviceSize buffer_image_granularity);

		void Release(const hrs::block<vk::DeviceSize> &blk);

		constexpr void SetResourceType(MemoryPoolResourceType _resource_type) noexcept;

		void Destroy(vk::Device device) noexcept;

	private:

		constexpr vk::DeviceSize round_up_required_alignment(vk::DeviceSize req_alignment,
															 MemoryPoolResourceType req_resource_type,
															 vk::DeviceSize buffer_image_granularity) noexcept;

		Memory memory;
		hrs::relaxed_free_block_chain<vk::DeviceSize> blocks;
		MemoryPoolResourceType resource_type;
	};

	inline MemoryPool::MemoryPool(Memory &&_memory,
								  MemoryPoolResourceType _resource_type)
		: resource_type(_resource_type),
		  blocks(_memory.size),
		  memory(std::move(_memory)) {}

	inline MemoryPool::MemoryPool(MemoryPool &&pool) noexcept
		: resource_type(pool.resource_type),
		  blocks(std::move(pool.blocks)),
		  memory(std::move(pool.memory)) {}

	constexpr vk::DeviceSize MemoryPool::GetSize() const noexcept
	{
		return memory.size;
	}

	constexpr MemoryPoolResourceType MemoryPool::GetResourceType() const noexcept
	{
		return resource_type;
	}

	constexpr void MemoryPool::SetResourceType(MemoryPoolResourceType _resource_type) noexcept
	{
		resource_type = _resource_type;
	}

	constexpr Memory MemoryPool::GetMemory() noexcept
	{
		return memory;
	}

	constexpr const Memory MemoryPool::GetMemory() const noexcept
	{
		return memory;
	}

	constexpr bool MemoryPool::IsEmpty() const noexcept
	{
		return blocks.is_empty();
	}

	inline vk::Result MemoryPool::MapMemory(vk::Device device) noexcept
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		if(memory.IsMapped())
			return vk::Result::eSuccess;

		auto res = device.mapMemory(memory.device_memory, 0, memory.size);
		if(res.result == vk::Result::eSuccess)
			memory.map_ptr = static_cast<std::byte *>(res.value);

		return res.result;
	}

	hrs::expected<std::optional<hrs::block<vk::DeviceSize>>, vk::Result>
	inline MemoryPool::Bind(const vk::MemoryRequirements &req,
							vk::Device device,
							vk::Buffer &buffer,
							vk::DeviceSize buffer_image_granularity)
	{
		hrs::assert_true_debug(hrs::is_power_of_two(buffer_image_granularity),
							   "Granularity={} is not a power of two!", buffer_image_granularity);
		hrs::assert_true_debug(buffer, "Buffer isn't created yet!");
		hrs::assert_true_debug(device, "Device isn't created yet!");

		vk::DeviceSize block_alignment = round_up_required_alignment(req.alignment,
																	 MemoryPoolResourceType::Linear,
																	 buffer_image_granularity);

		auto hint = blocks.find_place_no_append(req.size, block_alignment);
		if(!hint)
			return std::optional<hrs::block<vk::DeviceSize>>{};

		auto blk = blocks.acquire_by_hint(hint.value(), req.size, block_alignment);
		vk::Result res = device.bindBufferMemory(buffer, memory.device_memory, blk.offset);

		if(res != vk::Result::eSuccess)
		{
			blocks.release(blk);
			return res;
		}

		return std::optional<hrs::block<vk::DeviceSize>>{blk};
	}

	hrs::expected<std::optional<hrs::block<vk::DeviceSize>>, vk::Result>
	inline MemoryPool::Bind(const vk::MemoryRequirements &req,
							vk::Device device,
							vk::Image &image,
							MemoryPoolResourceType req_resource_type,
							vk::DeviceSize buffer_image_granularity)
	{
		hrs::assert_true_debug(hrs::is_power_of_two(buffer_image_granularity),
							   "Granularity={} is not a power of two!", buffer_image_granularity);
		hrs::assert_true_debug(image, "Image isn't created yet!");
		hrs::assert_true_debug(device, "Device isn't created yet!");

		vk::DeviceSize block_alignment = round_up_required_alignment(req.alignment,
																	 req_resource_type,
																	 buffer_image_granularity);

		auto hint = blocks.find_place_no_append(req.size, block_alignment);
		if(!hint)
			return std::optional<hrs::block<vk::DeviceSize>>{};

		auto blk = blocks.acquire_by_hint(hint.value(), req.size, block_alignment);
		vk::Result res = device.bindImageMemory(image, memory.device_memory, blk.offset);

		if(res != vk::Result::eSuccess)
		{
			blocks.release(blk);
			return res;
		}

		return std::optional<hrs::block<vk::DeviceSize>>{blk};
	}

	inline void MemoryPool::Release(const hrs::block<vk::DeviceSize> &blk)
	{
		blocks.release(blk);
		if(IsEmpty())
			resource_type = MemoryPoolResourceType::None;
	}

	inline void MemoryPool::Destroy(vk::Device device) noexcept
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");

		memory.Free(device);
		blocks.clear();
		resource_type = MemoryPoolResourceType::None;
	}

	constexpr vk::DeviceSize
	MemoryPool::round_up_required_alignment(vk::DeviceSize req_alignment,
											MemoryPoolResourceType req_resource_type,
											vk::DeviceSize buffer_image_granularity) noexcept
	{
		bool is_req_resource_type_same_as_pool_type_non_mixed =
			(req_resource_type == resource_type) &&
			(resource_type == MemoryPoolResourceType::Linear || resource_type == MemoryPoolResourceType::NonLinear);

		if(buffer_image_granularity != 1 && !is_req_resource_type_same_as_pool_type_non_mixed)
			return hrs::round_up_size_to_alignment(req_alignment, buffer_image_granularity);

		return req_alignment;
	}
};
