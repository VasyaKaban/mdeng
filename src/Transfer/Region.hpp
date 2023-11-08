#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include <variant>

namespace FireLand
{
	template<typename T>
		requires std::same_as<T, vk::BufferCopy> || std::same_as<T, vk::BufferImageCopy>
	struct RegionOffset
	{
		T region;
		std::size_t offset;
		constexpr RegionOffset(const T &_region = {}, std::size_t _offset = {})
			: region(_region), offset(_offset) {}
		constexpr RegionOffset(const RegionOffset &) = default;
		constexpr RegionOffset(RegionOffset &&) = default;
		constexpr auto operator=(const RegionOffset &) -> RegionOffset & = default;
		constexpr auto operator=(RegionOffset &&) -> RegionOffset & = default;
	};

	template<typename T>
		requires std::same_as<T, vk::BufferCopy> || std::same_as<T, vk::BufferImageCopy>
	struct RegionsOffsets
	{
		std::vector<T> regions;
		std::vector<std::size_t> regions_offsets;
		constexpr RegionsOffsets(std::vector<T> &&_regions = {},
									   std::vector<std::size_t> &&_regions_offsets = {})
			: regions(std::move(_regions)), regions_offsets(std::move(_regions_offsets)) {}
		constexpr RegionsOffsets(const RegionsOffsets &) = default;
		constexpr RegionsOffsets(RegionsOffsets &&) = default;
		constexpr auto operator=(const RegionsOffsets &) -> RegionsOffsets & = default;
		constexpr auto operator=(RegionsOffsets &&) -> RegionsOffsets & = default;
	};
};
