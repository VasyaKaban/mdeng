/**
 * @file
 *
 * Represents region classes for transfer
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include <variant>

namespace FireLand
{
	/**
	 * @brief The RegionOffset class
	 * @tparam T must be vk::BufferCopy or vk::BufferImageCopy
	 */
	template<typename T>
		requires std::same_as<T, vk::BufferCopy> || std::same_as<T, vk::BufferImageCopy>
	struct RegionOffset
	{
		T region;///<copy structure
		std::size_t offset;///<offset from start of data pointer(used in high-level structures)
		constexpr RegionOffset(const T &_region = {}, std::size_t _offset = {})
			: region(_region), offset(_offset) {}
		constexpr RegionOffset(const RegionOffset &) = default;
		constexpr RegionOffset(RegionOffset &&) = default;
		constexpr RegionOffset & operator=(const RegionOffset &) = default;
		constexpr RegionOffset & operator=(RegionOffset &&) = default;
	};

	/**
	 * @brief The RegionsOffsets class
	 * @tparam T must be vk::BufferCopy or vk::BufferImageCopy
	 */
	template<typename T>
		requires std::same_as<T, vk::BufferCopy> || std::same_as<T, vk::BufferImageCopy>
	struct RegionsOffsets
	{
		std::vector<T> regions;///<array of copy structures

		///array of offsets from start of data pointer(used in high-level structures)
		std::vector<std::size_t> regions_offsets;

		constexpr RegionsOffsets(std::vector<T> &&_regions = {},
									   std::vector<std::size_t> &&_regions_offsets = {})
			: regions(std::move(_regions)), regions_offsets(std::move(_regions_offsets)) {}
		constexpr RegionsOffsets(const RegionsOffsets &) = default;
		constexpr RegionsOffsets(RegionsOffsets &&) = default;
		constexpr RegionsOffsets & operator=(const RegionsOffsets &) = default;
		constexpr RegionsOffsets & operator=(RegionsOffsets &&) = default;
	};
};
