#pragma once

#include <list>
#include <cstdint>
#include <vector>
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "../Vulkan/VulkanInclude.h"
#include "../Vulkan/InitResult.h"

namespace FireLand
{
	class DescriptorPool;
	class DeviceLoader;

	struct DescriptorSetGroup
	{
		std::vector<VkDescriptorSet> sets;
		DescriptorPool *parent_pool;
	};

	struct DescriptorPoolInfo
	{
		VkDescriptorPoolCreateFlags flags;
		std::uint32_t max_sets;
		std::vector<VkDescriptorPoolSize> pool_sizes;
	};

	class DescriptorStorage : public hrs::non_copyable
	{
	private:
		DescriptorStorage(VkDevice _device,
						  const DeviceLoader *_dl,
						  const VkAllocationCallbacks *_allocation_callbacks,
						  std::vector<VkDescriptorSetLayout> &&_descriptor_set_layouts,
						  DescriptorPoolInfo &&_pool_info) noexcept;

	public:
		static hrs::expected<DescriptorStorage, InitResult>
		Create(VkDevice _device,
			   const DeviceLoader &_dl,
			   const VkDescriptorSetLayoutCreateInfo &set_layout_info,
			   std::uint32_t set_layout_count,
			   DescriptorPoolInfo &&_pool_info,
			   const VkAllocationCallbacks *_allocation_callbacks);

		~DescriptorStorage();
		DescriptorStorage(DescriptorStorage &&storage) noexcept;
		DescriptorStorage & operator=(DescriptorStorage &&storage) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;

		hrs::expected<DescriptorSetGroup, VkResult> AllocateSetGroup();
		void RetireSetGroup(const DescriptorSetGroup &group);
		void FreeSetGroup(const DescriptorSetGroup &group);

		void DestroyFoolPools() noexcept;

		VkDevice GetDevice() const noexcept;
		const DeviceLoader * GetDeviceLoader() const noexcept;
		VkDescriptorSetLayout GetDescriptorSetLayout() const noexcept;
		const std::vector<VkDescriptorSetLayout> & GetDescriptorSetLayouts() const noexcept;
		const DescriptorPoolInfo & GetPoolInfo() const noexcept;
		const VkAllocationCallbacks * GetAllocationCallbacks() const noexcept;
		std::uint32_t GetSetLayoutCount() const noexcept;

	private:
		VkDevice device;
		const DeviceLoader *dl;
		const VkAllocationCallbacks *allocation_callbacks;
		std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
		DescriptorPoolInfo pool_info;
		std::list<DescriptorPool> pools;
	};
};
