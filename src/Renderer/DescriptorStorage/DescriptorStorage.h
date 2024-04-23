#pragma once

#include <list>
#include <cstdint>
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	class Device;
	class DescriptorPool;

	struct DescriptorSetGroup
	{
		std::vector<vk::DescriptorSet> sets;
		DescriptorPool *parent_pool;

		DescriptorSetGroup(std::vector<vk::DescriptorSet> &&_sets = {}, DescriptorPool *_parent_pool = {}) noexcept
			: sets(std::move(_sets)), parent_pool(_parent_pool) {}

		auto operator<=>(const DescriptorSetGroup &) const noexcept = default;
	};

	struct DescriptorPoolInfo
	{
		vk::DescriptorPoolCreateFlags flags;
		std::uint32_t max_sets;
		std::vector<vk::DescriptorPoolSize> pool_sizes;
	};

	class DescriptorStorage : public hrs::non_copyable
	{
	public:
		DescriptorStorage(Device *_parent_device = {},
						  vk::DescriptorSetLayout _descriptor_set_layout = {},
						  std::uint32_t set_layout_count = {},
						  const DescriptorPoolInfo &_pool_info = {});
		DescriptorStorage(Device *_parent_device = {},
						  vk::DescriptorSetLayout _descriptor_set_layout = {},
						  std::uint32_t set_layout_count = {},
						  DescriptorPoolInfo &&_pool_info = {});

		static hrs::expected<DescriptorStorage, vk::Result>
		Create(Device *_parent_device,
			   const vk::DescriptorSetLayoutCreateInfo &set_layout_info,
			   std::uint32_t set_layout_count,
			   const DescriptorPoolInfo &_pool_info);

		static hrs::expected<DescriptorStorage, vk::Result>
		Create(Device *_parent_device,
			   const vk::DescriptorSetLayoutCreateInfo &set_layout_info,
			   std::uint32_t set_layout_count,
			   DescriptorPoolInfo &&_pool_info) noexcept;

		~DescriptorStorage();
		DescriptorStorage(DescriptorStorage &&storage) noexcept;
		DescriptorStorage & operator=(DescriptorStorage &&storage) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;

		hrs::expected<DescriptorSetGroup, vk::Result> AllocateSetGroup();
		void RetireSetGroup(const DescriptorSetGroup &group);
		void FreeSetGroup(const DescriptorSetGroup &group);

		void DestroyFoolPools() noexcept;

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		vk::DescriptorSetLayout GetDescriptorSetLayout() const noexcept;
		const DescriptorPoolInfo & GetPoolInfo() const noexcept;

	private:
		Device *parent_device;
		std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
		DescriptorPoolInfo pool_info;

		std::list<DescriptorPool> pools;
	};
};
