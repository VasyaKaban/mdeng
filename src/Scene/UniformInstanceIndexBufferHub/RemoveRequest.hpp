/**
 * @file
 *
 * Represents the RemoveIndicesRequest class
 */

#pragma once

#include "IndexPool.hpp"

namespace FireLand
{
	/**
	 * @brief The RemoveIndicesRequest class
	 *
	 * Used as remove request for UniformIndexAllocator
	 */
	struct RemoveIndicesRequest
	{
		IndexPool::IndexType offset;///<block offset in indices
		IndexPool::IndexType count;///<block size in indices

		/**
		 * @brief ToBlock
		 * @return converted to block request
		 */
		hrs::block<vk::DeviceSize> ToBlock() const noexcept
		{
			return {static_cast<vk::DeviceSize>(count * sizeof(IndexPool::IndexType)),
					static_cast<vk::DeviceSize>(offset * sizeof(IndexPool::IndexType))};
		}
	};
};
