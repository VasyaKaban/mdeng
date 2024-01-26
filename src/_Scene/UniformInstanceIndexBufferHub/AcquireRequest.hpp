/**
 * @file
 *
 * Represenst the AcquireIndicesRequest class
 */
#pragma once

#include "IndexPool.hpp"

namespace FireLand
{
	/**
	 * @brief The AcquireIndicesRequest class
	 *
	 * Used as acquire request for UniformIndexAllocator
	 */
	struct AcquireIndicesRequest
	{
		std::vector<IndexPool::IndexType> init_data;///<initial indices
		std::size_t power_size;///<power_size for IndexPool
		std::size_t power_mul;///power_mul for IndexPool

		/**
		 * @brief CalculateMaxCapacity
		 * @return capacity of pool based on initial data size and power_size
		 */
		vk::DeviceSize CalculateMaxCapacity() const noexcept
		{
			return std::max(static_cast<vk::DeviceSize>(init_data.size() * sizeof(IndexPool::IndexType)),
							static_cast<vk::DeviceSize>(power_size * power_mul * sizeof(IndexPool::IndexType)));
		}
	};

	/**
	 * @brief The AcquireRequestPrepared class
	 *
	 * Used as prepared acquire statement after buffer reallocation
	 */
	struct AcquireRequestPrepared
	{
		const AcquireIndicesRequest &acquire_req;///<initial acquire request
		hrs::block<vk::DeviceSize> blk;///<allocated block
	};
};
