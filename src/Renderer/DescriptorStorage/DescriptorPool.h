#pragma once

#include <vector>
#include <cstdint>
#include <span>
#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
	class DescriptorStorage;
	class DescriptorStorage;

	class DescriptorPool : public hrs::non_copyable
	{
	private:
		DescriptorPool(DescriptorStorage *_storage,
					   VkDescriptorPool _pool,
					   std::uint32_t _max_sets_count) noexcept;

	public:
		static hrs::expected<DescriptorPool, VkResult>
		Create(DescriptorStorage &_parent_storage,
			   const VkDescriptorPoolCreateInfo &info) noexcept;

		~DescriptorPool();
		DescriptorPool(DescriptorPool &&d_pool) noexcept;
		DescriptorPool & operator=(DescriptorPool &&d_pool) noexcept;

		void Destroy() noexcept;
		bool IsCreated() const noexcept;

		hrs::expected<std::vector<VkDescriptorSet>, VkResult> AllocateSets();

		void ResetPool() noexcept;
		void RetireSets(std::span<const VkDescriptorSet> sets);
		void FreeSets(std::span<const VkDescriptorSet> sets) noexcept;

		DescriptorStorage * GetStorage() noexcept;
		const DescriptorStorage * GetStorage() const noexcept;
		VkDescriptorPool GetHandle() const noexcept;
		std::uint32_t GetMaxSetCount() const noexcept;
		std::uint32_t GetIssuedSetCount() const noexcept;
		std::uint32_t GetFreeSetCount() const noexcept;

	private:
		DescriptorStorage *storage;
		VkDescriptorPool pool;
		std::vector<VkDescriptorSet> free_sets;
		std::uint32_t max_set_count;
		std::uint32_t issued_sets_count;//allocated without free
	};
};
