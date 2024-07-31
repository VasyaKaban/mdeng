#pragma once

#include "MemoryType.h"
#include "MemoryPool.h"
#include "AllocatorLoader.h"
#include "../Vulkan/InitResult.h"

namespace FireLand
{
	class BoundedBuffer;
	class BoundedImage;
	class BoundedBlock;

	class InstanceLoader;

	struct MultipleAllocateDesiredOptions
	{
		VkMemoryPropertyFlags memory_property;
		MemoryTypeSatisfyOp op;
		hrs::flags<AllocationFlags> flags;
	};

	class Allocator : public hrs::non_copyable
	{
	private:
		Allocator(VkDevice _device,
				  AllocatorLoader &&al,
				  VkDeviceSize _buffer_image_granularity,
				  std::vector<MemoryType> &&_memory_types,
				  std::function<NewPoolSizeCalculator> &&_pool_size_calc,
				  const VkAllocationCallbacks *_allocation_callbacks) noexcept;
	public:
		Allocator() noexcept;
		~Allocator();
		Allocator(Allocator &&allocator) noexcept;
		Allocator & operator=(Allocator &&allocator) noexcept;

		static hrs::expected<Allocator, InitResult>
		Create(VkDevice _device,
			   VkPhysicalDevice physical_device,
			   PFN_vkGetDeviceProcAddr device_vkGetDeviceProcAddr,
			   const InstanceLoader &il,
			   std::function<NewPoolSizeCalculator> &&_pool_size_calc,
			   const VkAllocationCallbacks *_allocation_callbacks);

		void Destroy() noexcept;

		VkDevice GetDevice() const noexcept;
		const AllocatorLoader & GetAllocatorLoader() const noexcept;
		const std::vector<MemoryType> & GetMemoryTypes() const noexcept;
		const std::function<NewPoolSizeCalculator> & GetPoolSizeCalculatorFunction() const noexcept;
		void SetPoolSizeCalculatorFunction(std::function<NewPoolSizeCalculator> &&_pool_new_calc);
		VkDeviceSize GetBufferImageGranularity() const noexcept;
		bool IsGranularityFree() const noexcept;

		bool IsCreated() const noexcept;

		hrs::expected<std::pair<BoundedBlock, std::size_t>, hrs::error>
		Allocate(const VkMemoryRequirements &req,
				 ResourceType res_type,
				 std::span<const MultipleAllocateDesiredOptions> desired,
				 const std::function<NewPoolSizeCalculator> &calc = nullptr);

		void Free(const BoundedBlock &bb,
				  MemoryPoolOnEmptyPolicy policy,
				  ResourceType res_type);

		hrs::expected<std::pair<BoundedBuffer, std::size_t>, hrs::error>
		Allocate(const VkBufferCreateInfo &info,
				 std::span<const MultipleAllocateDesiredOptions> desired,
				 const std::function<NewPoolSizeCalculator> &calc = nullptr);

		void Free(const BoundedBuffer &bounded_buffer,
				  MemoryPoolOnEmptyPolicy policy);

		hrs::expected<std::pair<BoundedImage, std::size_t>, hrs::error>
		Allocate(const VkImageCreateInfo &info,
				 std::span<const MultipleAllocateDesiredOptions> desired,
				 const std::function<NewPoolSizeCalculator> &calc = nullptr);

		void Free(const BoundedImage &bounded_image,
				  MemoryPoolOnEmptyPolicy policy);

	private:
		bool is_memory_type_satisfy(const MemoryType &mem_type,
									MemoryTypeSatisfyOp op,
									VkMemoryPropertyFlags desired_props,
									const VkMemoryRequirements &req) const noexcept;
	private:
		VkDevice device;
		AllocatorLoader loader;
		VkDeviceSize buffer_image_granularity;
		std::vector<MemoryType> memory_types;
		std::function<NewPoolSizeCalculator> pool_size_calc;
		const VkAllocationCallbacks *allocation_callbacks;
	};
};
