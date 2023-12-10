/**
 * @file
 *
 * Represents the UpdateIndicesRequest class
 */
#pragma once

#include "IndexPool.hpp"

namespace FireLand
{
	/**
	 * @brief The UpdateIndicesRequest class
	 *
	 * Used as update request for UniformIndexBuffer
	 */
	struct UpdateIndicesRequest
	{
		IndexPool &pool;///<pool where data is stored
		IndexPool::IndexType erase_start;///<index from which start to erase
		IndexPool::IndexType erase_count;///<count of indices to erase
		std::vector<IndexPool::IndexType> add_data;///<new data to append

		/**
		 * @brief ShrinkErase
		 * @param map_ptr mapped pointer of buffer where pool is stored
		 *
		 * Erases data bases on erase_start and erase_count and shrinks tail to head
		 */
		void ShrinkErase(std::byte *map_ptr) const noexcept
		{
			vk::DeviceSize pool_start = static_cast<vk::DeviceSize>(pool.GetOffset()
																	* sizeof(IndexPool::IndexType));
			vk::DeviceSize erase_start_abs = pool_start +
											 static_cast<vk::DeviceSize>(erase_start * sizeof(IndexPool::IndexType));
			vk::DeviceSize erase_end_abs =  erase_start_abs +
										   static_cast<vk::DeviceSize>(erase_count * sizeof(IndexPool::IndexType));

			/*
			 * start = 3
			 * count = 4
			 *       s       e
			   0 1 2 3 4 5 6 7 8 9
			   0 1 2 7 8 9 * * * *

			 */

			std::memcpy(map_ptr + erase_start_abs,
						map_ptr + erase_end_abs,
						(pool.GetSize() - (erase_start + erase_count)) * sizeof(IndexPool::IndexType));

			pool.size -= erase_count;
		}

		/**
		 * @brief AppendData
		 * @param map_ptr mapped pointer of buffer where pool is stored
		 *
		 * Appends add_data to pool
		 */
		void AppendData(std::byte *map_ptr) const noexcept
		{
			vk::DeviceSize pool_start = static_cast<vk::DeviceSize>(pool.GetOffset()
																	* sizeof(IndexPool::IndexType));
			vk::DeviceSize append_start = pool_start +
										  static_cast<vk::DeviceSize>(pool.GetSize() * sizeof(IndexPool::IndexType));

			std::memcpy(map_ptr + append_start,
						add_data.data(),
						add_data.size() * sizeof(IndexPool::IndexType));

			pool.size += add_data.size();
		}

		/**
		 * @brief CalculateNewCapacity
		 * @return calculated new capacity based on target pool size and add_data size
		 */
		IndexPool::IndexType CalculateNewCapacity() const noexcept
		{
			return pool.CalculateSize(pool.GetSize() + add_data.size());
		}

	#warning new capacity???
		/**
		 * @brief MoveAndAppendToNew
		 * @param map_ptr mapped pointer of buffer where pool is stored
		 * @param new_offset offset of newly allocated block
		 * @param new_capacity capacity(size) of newly allocated block
		 *
		 * Moves indices to new block and append add_data to it
		 */
		void MoveAndAppendToNew(std::byte *map_ptr,
								vk::DeviceSize new_offset,
								vk::DeviceSize new_capacity) const noexcept
		{
			vk::DeviceSize pool_start = static_cast<vk::DeviceSize>(pool.GetOffset() * sizeof(IndexPool::IndexType));
			std::memcpy(map_ptr + new_offset,
						map_ptr + pool_start,
						pool.GetSize() * sizeof(IndexPool::IndexType));

			vk::DeviceSize new_add_offset_abs = new_offset +
												static_cast<vk::DeviceSize>(add_data.size() * sizeof(IndexPool::IndexType));
			std::memcpy(map_ptr + new_add_offset_abs,
						add_data.data(),
						add_data.size() * sizeof(IndexPool::IndexType));

			pool.size += add_data.size();
			pool.parent_offset = new_offset / sizeof(IndexPool::IndexType);
			pool.capacity = new_capacity / sizeof(IndexPool::IndexType);
		}

		/**
		 * @brief CalculateFreeSizeAfterErase
		 * @return pool's new free size after erase operation
		 */
		IndexPool::IndexType CalculateFreeSizeAfterErase() const noexcept
		{
			return pool.GetFreeSize() + erase_count;
		}

	#warning new capacity???
		/**
		 * @brief EraseAndAppendToNew
		 * @param map_ptr mapped pointer of buffer where pool is stored
		 * @param new_offset offset of newly allocated block
		 * @param new_capacity capacity(size) of newly allocated block
		 *
		 * Makes erase operation but moves pieces that remain after erasing to new block with shrinking and
		 * appends add_data to new block
		 */
		void EraseAndAppendToNew(std::byte *map_ptr,
								 vk::DeviceSize new_offset,
								 vk::DeviceSize new_capacity) const noexcept
		{
			IndexPool::IndexType index_new_offset = new_offset / sizeof(IndexPool::IndexType);
			vk::DeviceSize pool_start = static_cast<vk::DeviceSize>(pool.GetOffset()
																	* sizeof(IndexPool::IndexType));
			vk::DeviceSize erase_start_abs = pool_start +
											 static_cast<vk::DeviceSize>(erase_start * sizeof(IndexPool::IndexType));
			vk::DeviceSize erase_end_abs = erase_start_abs +
										   static_cast<vk::DeviceSize>(erase_count * sizeof(IndexPool::IndexType));
			//copy pre start
			std::memcpy(map_ptr + new_offset,
						map_ptr + pool_start,
						erase_start_abs - pool_start);

			new_offset += (erase_start_abs - pool_start);
			//copy post end
			std::memcpy(map_ptr + new_offset,
						map_ptr + erase_end_abs,
						(pool.GetSize() - (erase_start + erase_count)) * sizeof(IndexPool::IndexType));

			new_offset += (pool.GetSize() - (erase_start + erase_count)) * sizeof(IndexPool::IndexType);
			//copy add_data
			std::memcpy(map_ptr + new_offset,
						add_data.data(),
						add_data.size() * sizeof(IndexPool::IndexType));

			pool.size += add_data.size();
			pool.parent_offset = index_new_offset;
			pool.capacity = new_capacity / sizeof(IndexPool::IndexType);
		}
	};

	/**
	 * @brief The UpdateRequestPrepared class
	 *
	 * Used as prepared acquire statement after buffer reallocation
	 */
	struct UpdateRequestPrepared
	{
		const UpdateIndicesRequest &update_req;///<initial update request
		hrs::block<vk::DeviceSize> blk;///<newly allocated block
	};
};
