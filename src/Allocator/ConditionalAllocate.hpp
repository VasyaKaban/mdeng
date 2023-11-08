#pragma once

#include "Allocator.hpp"

namespace FireLand
{
	enum class DesiredType
	{
		Any,
		Only
	};

	struct AllocateConditionlInfo
	{
		DesiredType type;
		vk::MemoryPropertyFlags property_flags;
		bool map_memory;
	};

	template<std::ranges::sized_range R>
		requires std::same_as<std::ranges::range_value_t<R>, AllocateConditionlInfo>
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
				res = allocator.AllocateBufferAny(info, alloc_info->property_flags, alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
			else
			{
				res = allocator.AllocateBufferOnly(info, alloc_info->property_flags, alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
		}

		return res.error();
	}

	template<std::ranges::sized_range R>
		requires std::same_as<std::ranges::range_value_t<R>, AllocateConditionlInfo>
	inline auto AllocateImageBuffer(Allocator &allocator,
									const R &rng,
									const vk::ImageCreateInfo &info,
									vk::DeviceSize size)
		-> hrs::expected<std::pair<ImageBuffer, decltype(std::ranges::begin(rng))>, AllocatorError>
	{
		if(std::ranges::size(rng) == 0)
			return AllocatorError(vk::Result::eErrorUnknown);

		hrs::expected<ImageBuffer, AllocatorError> res;
		for(auto alloc_info = std::ranges::begin(rng); alloc_info != std::ranges::end(rng); alloc_info++)
		{
			if(alloc_info->type == DesiredType::Any)
			{
				res = allocator.AllocateImageBufferAny(info,
													   size,
													   alloc_info->property_flags,
													   alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
			else
			{
				res = allocator.AllocateImageBufferOnly(info,
														size,
														alloc_info->property_flags,
														alloc_info->map_memory);
				if(res.has_value())
					return std::pair{res.value(), alloc_info};
			}
		}

		return res.error();
	}
};
