#pragma once

#include "../../hrs/expected.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../Vulkan/VulkanInclude.hpp"
#include <cstdint>

namespace FireLand
{
	class DescriptorStorage;

	class DescriptorPool : public hrs::non_copyable
	{
	public:
		DescriptorPool(DescriptorStorage *_parent_storage = {},
					   vk::DescriptorPool _pool = {},
					   std::uint32_t _max_sets_count = {},
					   std::uint32_t _issued_sets_count = {}) noexcept;

		static hrs::expected<DescriptorPool, vk::Result>
		Create(DescriptorStorage *_parent_storage, const vk::DescriptorPoolCreateInfo &info) noexcept;

		~DescriptorPool();
		DescriptorPool(DescriptorPool &&d_pool) noexcept;
		DescriptorPool & operator=(DescriptorPool &&d_pool) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;

		hrs::expected<std::vector<vk::DescriptorSet>, vk::Result> AllocateSets(std::uint32_t count);
		hrs::expected<std::vector<vk::DescriptorSet>, vk::Result>
		AllocateSets(std::span<const vk::DescriptorSetLayout> layouts);

		void ResetPool() noexcept;
		void RetireSets(std::span<const vk::DescriptorSet> sets);
		void FreeSets(std::span<const vk::DescriptorSet> sets) noexcept;

		DescriptorStorage * GetParentStorage() noexcept;
		const DescriptorStorage * GetParentStorage() const noexcept;
		vk::DescriptorPool GetHandle() const noexcept;
		std::uint32_t GetMaxSetCount() const noexcept;
		std::uint32_t GetIssuedSetCount() const noexcept;
		std::uint32_t GetFreeSetCount() const noexcept;

	private:
		DescriptorStorage *parent_storage;
		vk::DescriptorPool pool;

		std::vector<vk::DescriptorSet> free_sets;

		std::uint32_t max_sets_count;
		std::uint32_t issued_sets_count;
	};
};
