/**
 * @file
 *
 * Represents IndexPool class
 */

#pragma once

#include <cstdint>
#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/block.hpp"

namespace FireLand
{
	/**
	 * @brief The IndexPool class
	 *
	 * Used as vector of per-instance index to uniform data in uniform buffer
	 */
	struct IndexPool
	{
		using IndexType = std::uint32_t;

		/**
		 * @brief IndexPool
		 * @param _parent_offset offset within buffer
		 * @param _power_size granularity of size(size must be a power of power_size)
		 * @param power_mul multiplication factor of power_size
		 *
		 * Actual size calculated like:
		 * @code
		 *	size = power_size * power_mul;
		 * @endcode
		 */
		IndexPool(IndexType _parent_offset = 0,
				  IndexType _power_size = 0,
				  IndexType power_mul = 0);
		IndexPool(const IndexPool &) = delete;
		IndexPool(IndexPool &&pool) noexcept;

		IndexPool & operator=(const IndexPool &) = delete;
		IndexPool & operator=(IndexPool &&pool) noexcept;

		/**
		 * @brief IsFull
		 * @return return true if size equals to capacity
		 */
		constexpr bool IsFull() const noexcept;

		/**
		 * @brief GetOffset
		 * @return pool offset in parent buffer
		 */
		constexpr IndexType GetOffset() const noexcept;

		/**
		 * @brief GetSize
		 * @return occupied pool size
		 */
		constexpr IndexType GetSize() const noexcept;

		/**
		 * @brief GetCapacity
		 * @return capacity of pool
		 */
		constexpr IndexType GetCapacity() const noexcept;

		/**
		 * @brief GetFreeSize
		 * @return count of free places
		 */
		constexpr IndexType GetFreeSize() const noexcept;

		/**
		 * @brief CanAdd
		 * @param count desired count of index places
		 * @return true if pool has free places equal or more than count
		 */
		constexpr bool CanAdd(IndexType count) const noexcept;

		/**
		 * @brief GetPowerSize
		 * @return power size
		 */
		constexpr IndexType GetPowerSize() const noexcept;

		/**
		 * @brief CalculateSize
		 * @param desired_size size that is desired to be
		 * @return recalculated size with respect to power_size
		 */
		constexpr IndexType CalculateSize(IndexType desired_size) const noexcept;

		constexpr hrs::block<vk::DeviceSize> GetBlock() const noexcept
		{
			return {static_cast<vk::DeviceSize>(size * sizeof(IndexType)),
					static_cast<vk::DeviceSize>(parent_offset * sizeof(IndexType))};
		}

		IndexType parent_offset;///<pool offset in parent buffer
		IndexType size;///<already occupied size
		IndexType capacity;///<capacity of pool
		IndexType power_size;///<needs for reallocations to take capacity fit for feature adds
	};

	IndexPool::IndexPool(IndexType _parent_offset,
						 IndexType _power_size,
						 IndexType power_mul)
	{
		parent_offset = _parent_offset;
		size = 0;
		power_size = _power_size;
		capacity = power_mul * _power_size;
	}

	IndexPool::IndexPool(IndexPool &&pool) noexcept
		: parent_offset(pool.parent_offset),
		  size(pool.size),
		  capacity(pool.capacity),
		  power_size(pool.power_size) {}

	IndexPool & IndexPool::operator=(IndexPool &&pool) noexcept
	{
		parent_offset = pool.parent_offset;
		size = pool.size;
		capacity = pool.capacity;
		power_size = pool.power_size;

		return *this;
	}

	constexpr bool IndexPool::IsFull() const noexcept
	{
		return size == capacity;
	}

	constexpr IndexPool::IndexType IndexPool::GetOffset() const noexcept
	{
		return parent_offset;
	}

	constexpr IndexPool::IndexType IndexPool::GetSize() const noexcept
	{
		return size;
	}

	constexpr IndexPool::IndexType IndexPool::GetCapacity() const noexcept
	{
		return capacity;
	}

	constexpr IndexPool::IndexType IndexPool::GetFreeSize() const noexcept
	{
		return capacity - size;
	}

	constexpr bool IndexPool::CanAdd(IndexType count) const noexcept
	{
		return (capacity - size) >= count;
	}

	constexpr IndexPool::IndexType IndexPool::GetPowerSize() const noexcept
	{
		return power_size;
	}

	constexpr IndexPool::IndexType IndexPool::CalculateSize(IndexType desired_size) const noexcept
	{
		if(desired_size % power_size == 0)
			return desired_size;

		return (desired_size / power_size + 1) * power_size;
	}
};

