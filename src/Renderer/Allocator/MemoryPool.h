#pragma once

#include "../Vulkan/VulkanInclude.h"
#include "hrs/non_creatable.hpp"
#include "hrs/sized_free_block_chain.hpp"
#include "hrs/error.hpp"
#include "hrs/expected.hpp"
#include "Memory.h"
#include "AllocatorResult.h"

namespace FireLand
{
	enum class ResourceType
	{
		Linear,
		NonLinear
	};

	enum class MemoryPoolType
	{
		None = 0,
		Linear = 1,
		NonLinear = 2,
		Mixed = 3,
		MemoryPoolTypeMaxUnused
	};

	class MemoryPool
		: public hrs::non_copyable,
		  public hrs::non_move_assignable
	{
	private:
		MemoryPool(VkDeviceSize _buffer_image_granularity,
				   Memory &&_memory,
				   hrs::sized_free_block_chain<VkDeviceSize> &&_free_blocks) noexcept;
	public:
		MemoryPool() noexcept;
		~MemoryPool() = default;
		MemoryPool(MemoryPool &&pool) noexcept;

		static MemoryPoolType ToMemoryPoolType(ResourceType res_type) noexcept;

		static hrs::expected<MemoryPool, VkResult>
		Create(VkDevice device,
			   VkDeviceSize size,
			   std::uint32_t memory_type_index,
			   bool map_memory,
			   VkDeviceSize _buffer_image_granularity,
			   const DeviceLoader &dl,
			   const VkAllocationCallbacks *alc);

		bool IsCreated() const noexcept;
		void Destroy(VkDevice device, const DeviceLoader &dl, const VkAllocationCallbacks *alc) noexcept;

		Memory & GetMemory() noexcept;
		const Memory & GetMemory() const noexcept;

		VkDeviceSize GetGranularity() const noexcept;
		MemoryPoolType GetType() const noexcept;
		bool IsGranularityFree() const noexcept;
		bool IsEmpty() const noexcept;

		std::size_t GetNonLinearObjectCount() const noexcept;
		std::size_t GetLinearObjectCount() const noexcept;

		hrs::expected<std::byte *, VkResult> MapMemory(VkDevice device, const DeviceLoader &dl) noexcept;

		hrs::expected<hrs::block<VkDeviceSize>, AllocatorResult>
		Acquire(ResourceType res_type, const hrs::mem_req<VkDeviceSize> &req);

		void Release(ResourceType res_type, const hrs::block<VkDeviceSize> &blk) noexcept;

	private:

		void inc_count(ResourceType res_type) noexcept;

		void dec_count(ResourceType res_type) noexcept;

		std::optional<hrs::block<VkDeviceSize>>
		acquire_block_based_on_granularity(ResourceType res_type,
										   hrs::mem_req<VkDeviceSize> req);

	private:
		VkDeviceSize buffer_image_granularity;
		std::size_t non_linear_object_count;
		std::size_t linear_object_count;
		Memory memory;
		hrs::sized_free_block_chain<VkDeviceSize> free_blocks;
	};
};
