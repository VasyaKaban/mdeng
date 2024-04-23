#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/flags.hpp"
#include "hrs/error.hpp"
#include "hrs/block.hpp"
#include "hrs/expected.hpp"
#include <list>
#include <cstdint>
#include <functional>
#include "MemoryPoolLists.h"

namespace FireLand
{
	class MemoryPool;
	enum class MemoryPoolResult;
	enum class ResourceType;

	enum class MemoryPoolOnEmptyPolicy
	{
		Keep,
		Free,
	};

	enum class AllocationFlags
	{
		AllowPlaceWithMixedResources = 1 << 0,
		AllocateSeparatePool = 1 << 1,
		MapMemory = 1 << 2,
		CreateOnExistedPools = 1 << 3
	};

	struct BlockBindPool
	{
		hrs::block<vk::DeviceSize> block;
		MemoryPoolLists::Iterator pool;

		BlockBindPool(const hrs::block<vk::DeviceSize> &_block = {}, MemoryPoolLists::Iterator _pool = {}) noexcept
			: block(_block), pool(_pool) {}
		BlockBindPool(const BlockBindPool &) = default;
		BlockBindPool & operator=(const BlockBindPool &) = default;
	};

	class MemoryType;

	//return value must be greater or equal to requested_size if it's supposed to be a good allocation size
	//otherwise it's supposed to be an allocation failure
	using NewPoolSizeCalculator = vk::DeviceSize(vk::DeviceSize /*previous_size -> 0 on first call, returned value for next calls*/,
												 vk::DeviceSize /*requested_size -> requested size for allocation(passes on first call)*/,
												 const MemoryType & /*mem_type -> memory type where pool wiil be placed*/);


	class MemoryType : public hrs::non_copyable
	{
	public:

		using PoolContainer = std::list<MemoryPool>;

		static vk::DeviceSize DefaultNewPoolSizeCalculator(vk::DeviceSize previous_size,
														   vk::DeviceSize requested_size,
														   const MemoryType &mem_type) noexcept;

		MemoryType(vk::Device _parent_device,
				   vk::MemoryHeap _heap,
				   vk::MemoryPropertyFlags _memory_property_flags,
				   std::uint32_t _index,
				   vk::DeviceSize _buffer_image_granularity) noexcept;

		~MemoryType();
		MemoryType(MemoryType &&mem_type) noexcept;
		MemoryType & operator=(MemoryType &&mem_type) noexcept;

		void Destroy();

		bool IsSatisfy(const vk::MemoryRequirements &req) const noexcept;
		bool IsSatisfyAny(vk::MemoryPropertyFlags props) const noexcept;
		bool IsSatisfyOnly(vk::MemoryPropertyFlags props) const noexcept;
		bool IsMappable() const noexcept;
		bool IsEmpty() const noexcept;

		std::uint32_t GetMemoryTypeIndex() const noexcept;
		std::uint32_t GetMemoryTypeIndexMask() const noexcept;

		hrs::expected<BlockBindPool, hrs::error>
		Bind(vk::Buffer buffer,
			 const vk::MemoryRequirements &req,
			 hrs::flags<AllocationFlags> flags = {},
			 const std::function<NewPoolSizeCalculator> &calc = DefaultNewPoolSizeCalculator);

		hrs::expected<BlockBindPool, hrs::error>
		Bind(vk::Image image,
			 ResourceType res_type,
			 const vk::MemoryRequirements &req,
			 hrs::flags<AllocationFlags> flags = {},
			 const std::function<NewPoolSizeCalculator> &calc = DefaultNewPoolSizeCalculator);

		void Release(ResourceType res_type,
					 const BlockBindPool &bbp,
					 MemoryPoolOnEmptyPolicy policy);

		void Release(vk::Buffer buffer,
					 const BlockBindPool &bbp,
					 MemoryPoolOnEmptyPolicy policy);

		void Release(vk::Image image,
					 ResourceType res_type,
					 const BlockBindPool &bbp,
					 MemoryPoolOnEmptyPolicy policy);

	private:
		hrs::expected<BlockBindPool, hrs::error>
		bind(std::variant<vk::Buffer, vk::Image> resource,
			 ResourceType res_type,
			 const vk::MemoryRequirements &req,
			 hrs::flags<AllocationFlags> flags,
			 const std::function<NewPoolSizeCalculator> &calc);

		hrs::expected<hrs::block<vk::DeviceSize>, hrs::error>
		bind_to_pool(MemoryPool &pool,
					 std::variant<vk::Buffer, vk::Image> resource,
					 ResourceType res_type,
					 const vk::MemoryRequirements &req);

		hrs::expected<BlockBindPool, vk::Result>
		allocate_and_bind(std::variant<vk::Buffer, vk::Image> resource,
						  ResourceType res_type,
						  const vk::MemoryRequirements &req,
						  hrs::flags<AllocationFlags> flags,
						  const std::function<NewPoolSizeCalculator> &calc);

		hrs::expected<BlockBindPool, hrs::error>
		bind_to_existed(std::variant<vk::Buffer, vk::Image> resource,
						ResourceType res_type,
						const vk::MemoryRequirements &req,
						hrs::flags<AllocationFlags> flags);

		hrs::expected<BlockBindPool, hrs::error>
		try_bind_to_existed_pools(MemoryPoolLists::PoolContainer &pools,
								  MemoryPoolType pool_type,
								  std::variant<vk::Buffer, vk::Image> resource,
								  ResourceType res_type,
								  const vk::MemoryRequirements &req);

	private:
		vk::Device parent_device;
		vk::MemoryHeap heap;
		vk::MemoryPropertyFlags memory_property_flags;
		std::uint32_t index;
		vk::DeviceSize buffer_image_granularity = 1;
		MemoryPoolLists lists;
	};
};
