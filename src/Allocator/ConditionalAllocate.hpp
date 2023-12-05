/**
 * @file
 *
 * Represents special functions for allocation
 */


#pragma once

#include "Allocator.hpp"

namespace FireLand
{
	/**
	 * @brief The DesiredType enum
	 */
	enum class DesiredType
	{
		Any,///<Any operation for desired flags
		Only///<Only operation for desired flags
	};

	/**
	 * @brief The AllocateConditionlInfo class
	 *
	 * Used for conditional allocations
	 */
	struct AllocateConditionalInfo
	{
		DesiredType type;///<operation
		vk::MemoryPropertyFlags property_flags;///<desired flags
		bool map_memory;///<flag for memory mapping
	};

	/**
	 * @brief AllocateBuffer
	 * @tparam R must satisfy sized_range concept and value type must be AllocateConditionlInfo
	 * @param allocator allocator from which allocate memory
	 * @param rng range of AllocateConditionlInfo structures
	 * @param info buffer create info
	 * @return a pair of allocated buffer and iterator with choosed AllocateConditionlInfo or AllocatorError
	 */
	template<std::ranges::sized_range R>
		requires std::same_as<std::ranges::range_value_t<R>, AllocateConditionalInfo>
	inline auto AllocateBuffer(Allocator &allocator, const R &rng, const vk::BufferCreateInfo &info)
		-> hrs::expected<std::pair<Buffer, decltype(std::ranges::begin(rng))>, AllocatorError>
	{
		if(std::ranges::size(rng) == 0)
			return AllocatorError(vk::Result::eErrorUnknown);

		hrs::expected<Buffer, AllocatorError> res;
		for(auto alloc_info = std::ranges::begin(rng); alloc_info != std::ranges::end(rng); alloc_info++)
		{
			if(alloc_info->type == DesiredType::Any)
			{
				res = allocator.AllocateAny(alloc_info->property_flags, info, alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
			else
			{
				res = allocator.AllocateOnly(alloc_info->property_flags, info, alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
		}

		return res.error();
	}

	/**
	 * @brief AllocateImageBuffer
	 * @tparam R must satisfy sized_range concept and value type must be AllocateConditionlInfo
	 * @param allocator allocator from which allocate memory
	 * @param rng range of AllocateConditionlInfo structures
	 * @param info image create info
	 * @param size size of memory
	 * @return a pair of allocated memory and iterator with choosed AllocateConditionlInfo or AllocatorError
	 */
	template<std::ranges::sized_range R>
		requires std::same_as<std::ranges::range_value_t<R>, AllocateConditionalInfo>
	inline auto AllocateImageBuffer(Allocator &allocator,
									const R &rng,
									const vk::ImageCreateInfo &info,
									vk::DeviceSize size)
		-> hrs::expected<std::pair<Memory, decltype(std::ranges::begin(rng))>, AllocatorError>
	{
		if(std::ranges::size(rng) == 0)
			return AllocatorError(vk::Result::eErrorUnknown);

		hrs::expected<Memory, AllocatorError> res;
		for(auto alloc_info = std::ranges::begin(rng); alloc_info != std::ranges::end(rng); alloc_info++)
		{
			if(alloc_info->type == DesiredType::Any)
			{
				res = allocator.AllocateAny(alloc_info->property_flags,
											info,
											size,
											alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
			else
			{
				res = allocator.AllocateOnly(alloc_info->property_flags,
											 info,
											 size,
											 alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
		}

		return res.error();
	}
};
