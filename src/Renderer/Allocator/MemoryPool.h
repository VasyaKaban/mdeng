#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/sized_free_block_chain.hpp"
#include "hrs/error.hpp"
#include "hrs/expected.hpp"
#include "Memory.h"

namespace FireLand
{
	enum class MemoryPoolType
	{
		None = 0,
		Linear,
		NonLinear,
		Mixed
	};

	enum class ResourceType
	{
		Linear,
		NonLinear
	};

	constexpr MemoryPoolType ResourceTypeToMemoryPoolType(ResourceType res_type) noexcept
	{
		return (res_type == ResourceType::Linear ? MemoryPoolType::Linear : MemoryPoolType::NonLinear);
	}

	enum class MemoryPoolResult
	{
		Success,
		NotEnoughSpace
	};

	constexpr auto MemoryPoolResultToString(MemoryPoolResult res) noexcept
	{
		switch(res)
		{
			case MemoryPoolResult::Success:
				return "Success";
				break;
			case MemoryPoolResult::NotEnoughSpace:
				return "NotEnoughSpace";
				break;
		}
	}

	class MemoryPool : public hrs::non_copyable
	{
		void init(Memory &&_memory,
				  vk::DeviceSize _buffer_image_granularity,
				  hrs::sized_free_block_chain<vk::DeviceSize> &&_free_blocks) noexcept;

	public:
		MemoryPool(vk::Device _parent_device) noexcept;
		~MemoryPool();
		MemoryPool(MemoryPool &&pool) noexcept;
		MemoryPool & operator=(MemoryPool &&pool) noexcept;

		vk::Result Recreate(vk::DeviceSize size,
							std::uint32_t memory_type_index,
							bool map_memory,
							vk::DeviceSize _buffer_image_granularity);

		bool IsCreated() const noexcept;
		void Destroy() noexcept;

		Memory & GetMemory() noexcept;
		const Memory & GetMemory() const noexcept;
		vk::Device GetParentDevice() const noexcept;

		vk::DeviceSize GetGranularity() const noexcept;
		MemoryPoolType GetType() const noexcept;
		bool IsGranularityFree() const noexcept;
		bool IsEmpty() const noexcept;

		std::size_t GetNonLinearObjectCount() const noexcept;
		std::size_t GetLinearObjectCount() const noexcept;

		vk::ResultValue<std::byte *> MapMemory() noexcept;

		void Release(ResourceType res_type, const hrs::block<vk::DeviceSize> &blk);
		void Release(vk::Buffer buffer, const hrs::block<vk::DeviceSize> &blk);
		void Release(vk::Image image, ResourceType res_type, const hrs::block<vk::DeviceSize> &blk);

		hrs::expected<hrs::block<vk::DeviceSize>, hrs::error>
		Bind(vk::Buffer buffer,
			 hrs::mem_req<vk::DeviceSize> req) noexcept;

		hrs::expected<hrs::block<vk::DeviceSize>, hrs::error>
		Bind(vk::Image image,
			 ResourceType res_type,
			 hrs::mem_req<vk::DeviceSize> req) noexcept;

	private:

		void inc_count(ResourceType res_type) noexcept;

		void dec_count(ResourceType res_type) noexcept;

		std::optional<hrs::block<vk::DeviceSize>>
		acquire_block_based_on_granularity(ResourceType res_type,
										   hrs::mem_req<vk::DeviceSize> req);

	private:
		vk::Device parent_device;
		vk::DeviceSize buffer_image_granularity = 1;
		std::size_t non_linear_object_count = {};
		std::size_t linear_object_count = {};
		Memory memory;
		hrs::sized_free_block_chain<vk::DeviceSize> free_blocks;
	};
};

template<>
struct hrs::enum_error_traits<FireLand::MemoryPoolResult>
{
	constexpr static void traits_hint() noexcept {};

	constexpr static std::string_view get_name(FireLand::MemoryPoolResult value) noexcept
	{
		switch(value)
		{
			case FireLand::MemoryPoolResult::Success:
				return "Success";
				break;
			case FireLand::MemoryPoolResult::NotEnoughSpace:
				return "NotEnoughSpace";
				break;
			default:
				return "";
				break;
		}
	}
};
