/**
 * @file
 *
 * Represents the UniformInstanceIndexAllocator class
 */

#pragma once

#include "AcquireRequest.hpp"
#include "RemoveRequest.hpp"
#include "UpdateRequest.hpp"
#include "../../hrs/free_block_allocator.hpp"
#include "../../hrs/expected.hpp"
#include "../../Allocator/Buffer.hpp"

namespace FireLand
{
	/**
	 * @brief The UniformInstanceIndexAllocatorResult class
	 *
	 * Used as result from UnformIndexAllocators's HandleRequests function
	 */
	struct UniformInstanceIndexAllocatorResult
	{
		/**
		 * @brief The State enum
		 *
		 * Used as state of buffer after requested operations
		 */
		enum class State
		{
			None,///<buffer remains the same
			Resized///<buffer was resized
		} state;///<the state of buffer

		std::vector<IndexPool> added_pools;///<vector of added pools one per each acquire request
	};

	/**
	 * @brief BufferAllocFunction
	 *
	 * Concept that specifies buffer allocation function that
	 * must take vk::DesviceSize(size of buffer) as a parameter
	 * and should return expected result of allocated buffer or error result
	 */
	template<typename F>
	concept BufferAllocFunction = std::invocable<F, vk::DeviceSize> &&
								  std::same_as<std::invoke_result_t<F, vk::DeviceSize>,
											   hrs::expected<Buffer, vk::Result>>;

	/**
	 * @brief The UniformInstanceIndexAllocatorRequest class
	 * @tparam F must satisfy BufferAllocFunction concept
	 *
	 * Used as request structure for UniformInstanceIndexAllocator
	 */
	template<BufferAllocFunction F>
	struct UniformInstanceIndexAllocatorRequest
	{
		Buffer &buffer;///<affected buffer
		F alloc_func;///<allocation function
		vk::Device device;///<device(owner of buffer)

		std::vector<RemoveIndicesRequest> remove_requests;///<remove requests
		std::vector<AcquireIndicesRequest> add_requests;///<acquire requests
		std::vector<UpdateIndicesRequest> update_requests;///<update requests
	};

	/**
	 * @brief The UniformInstanceIndexAllocator class
	 *
	 * Used as a free block buffer for IndexPools
	 */
	class UniformInstanceIndexAllocator
	{
	public:
		/**
		 * @brief UniformInstanceIndexAllocator
		 * @param indices_count size of buffer in indices
		 *
		 * Creates free block buffer that can store indice_count indices
		 */
		UniformInstanceIndexAllocator(std::size_t indices_count);

		~UniformInstanceIndexAllocator() = default;
		UniformInstanceIndexAllocator(const UniformInstanceIndexAllocator &ualloc);
		UniformInstanceIndexAllocator(UniformInstanceIndexAllocator &&ualloc) noexcept;
		UniformInstanceIndexAllocator & operator=(const UniformInstanceIndexAllocator &ualloc);
		UniformInstanceIndexAllocator & operator=(UniformInstanceIndexAllocator &&ualloc) noexcept;

		/**
		 * @brief GetSize
		 * @return size of inner buffer
		 */
		constexpr vk::DeviceSize GetSize() const noexcept;

		/**
		 * @brief GetFreeIndicesCount
		 * @return count of free indices
		 */
		constexpr std::size_t GetFreeIndicesCount() const noexcept;

		/**
		 * @brief HandleRequests
		 * @tparam F must satisfy BufferAllocFunction concept
		 * @param request structure of requests
		 * @param buffer affected buffer
		 * @return result of request handling or error result
		 */
		template<BufferAllocFunction F>
		hrs::expected<UniformInstanceIndexAllocatorResult, vk::Result>
		HandleRequests(const UniformInstanceIndexAllocatorRequest<F> &request);

		/**
		 * @brief Clear
		 * @param indices_count new size of inner buffer in indices
		 */
		void Clear(std::size_t indices_count);

	private:

		/**
		 * @brief remove_blocks
		 * @param request vector of remove requests
		 *
		 * Makes remove operations
		 */
		void remove_blocks(const std::vector<RemoveIndicesRequest> &request) noexcept;

		/**
		 * @brief acquire_blocks
		 * @param request vector of acquire requests
		 * @param buffer affected buffer
		 * @return appended to buffer size and prepared acquire requests
		 *
		 * Makes acqisitions of blocks and returns an array of them
		 */
		std::pair<vk::DeviceSize, std::vector<AcquireRequestPrepared>>
		acquire_blocks(const std::vector<AcquireIndicesRequest> &request, Buffer &buffer) noexcept;

		/**
		 * @brief update_blocks
		 * @param common_size target appended size
		 * @param request vector of update requests
		 * @param buffer affected buffer
		 * @return newly appended to buffer size and prepared update aqcuire requests
		 *
		 * Makes update operations
		 */
		std::pair<vk::DeviceSize, std::vector<UpdateRequestPrepared>>
		update_blocks(vk::DeviceSize common_size,
					  const std::vector<UpdateIndicesRequest> &request,
					  Buffer &buffer) noexcept;

		/**
		 * @brief update_no_erase_acquire
		 * @param common_size target appended size
		 * @param request update request
		 * @param buffer affected buffer
		 * @return newly appended to buffer size and prepared update aqcuire request
		 *
		 * Makes update operation without erasing but with acquiring
		 */
		std::pair<vk::DeviceSize, UpdateRequestPrepared>
		update_no_erase_acquire(vk::DeviceSize common_size,
								const UpdateIndicesRequest &request,
								Buffer &buffer) noexcept;

		/**
		 * @brief update_erase_no_acquire
		 * @param request update request
		 * @param buffer affected buffer
		 *
		 * Makes update operation with erasing but without acquiring
		 */
		void update_erase_no_acquire(const UpdateIndicesRequest &request, Buffer &buffer) noexcept;

		/**
		 * @brief update_erase_acquire
		 * @param common_size target appended size
		 * @param request update request
		 * @param buffer affected buffer
		 * @return newly appended to buffer size and prepared update aqcuire request
		 *
		 * Makes update operation with erasing and with acquiring
		 */
		std::pair<vk::DeviceSize, UpdateRequestPrepared>
		update_erase_acquire(vk::DeviceSize common_size,
							 const UpdateIndicesRequest &request,
							 Buffer &buffer) noexcept;

		hrs::free_block_allocator<vk::DeviceSize> blocks;///<free blocks for pools
	};

	UniformInstanceIndexAllocator::UniformInstanceIndexAllocator(std::size_t indices_count)
		: blocks(sizeof(IndexPool::IndexType), indices_count) {}

	UniformInstanceIndexAllocator::UniformInstanceIndexAllocator(const UniformInstanceIndexAllocator &ualloc)
		: blocks(ualloc.blocks) {}

	UniformInstanceIndexAllocator::UniformInstanceIndexAllocator(UniformInstanceIndexAllocator &&ualloc) noexcept
		: blocks(std::move(ualloc.blocks)) {}

	UniformInstanceIndexAllocator & UniformInstanceIndexAllocator::operator=(const UniformInstanceIndexAllocator &ualloc)
	{
		blocks = ualloc.blocks;

		return *this;
	}

	UniformInstanceIndexAllocator & UniformInstanceIndexAllocator::operator=(UniformInstanceIndexAllocator &&ualloc) noexcept
	{
		blocks = std::move(ualloc.blocks);

		return *this;
	}

	constexpr vk::DeviceSize UniformInstanceIndexAllocator::GetSize() const noexcept
	{
		return blocks.size();
	}

	constexpr std::size_t UniformInstanceIndexAllocator::GetFreeIndicesCount() const noexcept
	{
		return blocks.get_free_blocks_count();
	}

	/**
	 * @warning Aborts if affected buffer size isn't equal to target blocks size and
	 * if affected buffer memory isn't mapped to host and
	 * if device isn't created!
	 */
	template<BufferAllocFunction F>
	hrs::expected<UniformInstanceIndexAllocatorResult, vk::Result>
	UniformInstanceIndexAllocator::HandleRequests(const UniformInstanceIndexAllocatorRequest<F> &request)
	{
		hrs::assert_true_debug(request.buffer.memory.size == blocks.size(),
							   "The buffer size is different from the blocks size!");
		hrs::assert_true_debug(request.buffer.memory.map_ptr,
							   "Buffer memory isn't mapped!");
		hrs::assert_true_debug(request.device,
							   "Device sin't created yet!");

		UniformInstanceIndexAllocatorResult result;
		result.state = UniformInstanceIndexAllocatorResult::State::None;
		result.added_pools.reserve(request.add_requests.size());

		remove_blocks(request.remove_requests, request.buffer);
		auto [common_size, prepared_acquisitions] = acquire_blocks(request.add_requests, request.buffer);
		auto [new_common_size, prepared_update_acquisitions] = update_blocks(common_size,
																			 request.update_requests,
																			 request.buffer);

		if(new_common_size != 0)
		{
			result.state = UniformInstanceIndexAllocatorResult::State::Resized;
			auto vault_buffer = std::move(request.buffer);
			hrs::expected<Buffer, vk::Result> buffer_alloc_result = request.alloc_func(new_common_size);
			if(!buffer_alloc_result)
				return buffer_alloc_result.error();

			request.buffer = std::move(buffer_alloc_result.value());

			std::memcpy(request.buffer.memory.map_ptr,
						vault_buffer.memory.map_ptr,
						vault_buffer.memory.size);
			vault_buffer.Destroy(request.device);
		}

		for(const auto &acq : prepared_acquisitions)
		{
			std::memcpy(request.buffer.memory.map_ptr + acq.blk.offset,
						acq.acquire_req.init_data.data(),
						acq.acquire_req.init_data.size() * sizeof(IndexPool::IndexType));

			IndexPool pool;
			pool.parent_offset = acq.blk.offset / sizeof(IndexPool::IndexType);
			pool.capacity = acq.blk.size / sizeof(IndexPool::IndexType);
			pool.size = acq.acquire_req.init_data.size();
			pool.power_size = acq.blk.size / sizeof(IndexPool::IndexType) / acq.acquire_req.power_mul;
			result.added_pools.push_back(std::move(pool));
		}

		//write
		for(const auto &acq : prepared_update_acquisitions)
		{
			//move old indices to new block
			//append data to new block
			//release old block

			auto old_blk = acq.update_req.pool.GetBlock();
			acq.update_req.MoveAndAppendToNew(request.buffer.memory.map_ptr, acq.blk.offset, acq.blk.size);
			blocks.release_block(old_blk);
		}

		return result;
	}

	void UniformInstanceIndexAllocator::Clear(std::size_t indices_count)
	{
		blocks.clear(sizeof(IndexPool::IndexType), indices_count);
	}

	void UniformInstanceIndexAllocator::remove_blocks(const std::vector<RemoveIndicesRequest> &request) noexcept
	{
		for(const auto &rem_blk : request)
			blocks.release_block(rem_blk.ToBlock());
	}

	std::pair<vk::DeviceSize, std::vector<AcquireRequestPrepared>>
	UniformInstanceIndexAllocator::acquire_blocks(const std::vector<AcquireIndicesRequest> &request, Buffer &buffer) noexcept
	{
		vk::DeviceSize common_size = 0;
		std::vector<AcquireRequestPrepared> prepared;
		prepared.reserve(request.size());
		for(const auto &acq_req : request)
		{
			vk::DeviceSize capacity = acq_req.CalculateMaxCapacity();
			auto [appended_blocks_count, block] = blocks.acquire_blocks(capacity, 1);
			common_size += appended_blocks_count * blocks.get_block_size();
			prepared.push_back(AcquireRequestPrepared{acq_req, block});
		}

		return {common_size, prepared};
	}

	std::pair<vk::DeviceSize, std::vector<UpdateRequestPrepared>>
	UniformInstanceIndexAllocator::update_blocks(vk::DeviceSize common_size,
									  const std::vector<UpdateIndicesRequest> &request,
									  Buffer &buffer) noexcept
	{
		std::vector<UpdateRequestPrepared> prepared;
		prepared.reserve(request.size());
		for(const auto &upd_req : request)
		{
			if(upd_req.erase_count == 0)
			{
				//no erase
				if(!upd_req.add_data.empty())
				{
					auto [new_common_size, prep] = update_no_erase_acquire(common_size, upd_req, buffer);
					if(common_size != new_common_size)
					{
						prepared.push_back(std::move(prep));
						common_size = new_common_size;
					}
				}
			}
			else
			{
				//with erase
				if(upd_req.add_data.empty())
				{
					//no acquire
					update_erase_no_acquire(upd_req, buffer);
				}
				else
				{
					//with acquire
					auto [new_common_size, prep] = update_erase_acquire(common_size, upd_req, buffer);
					if(common_size != new_common_size)
					{
						prepared.push_back(std::move(prep));
						common_size = new_common_size;
					}
				}
			}
		}

		return {common_size, std::move(prepared)};
	}

#warning RELEASE BLOCK!!!
	std::pair<vk::DeviceSize, UpdateRequestPrepared>
	UniformInstanceIndexAllocator::update_no_erase_acquire(vk::DeviceSize common_size,
												const UpdateIndicesRequest &request,
												Buffer &buffer) noexcept
	{
		if(request.pool.CanAdd(request.add_data.size()))
		{
			//enough space
			request.AppendData(buffer.memory.map_ptr);
			return {common_size, UpdateRequestPrepared{request, {}}};
		}
		else
		{
			//not enough space
			IndexPool::IndexType new_capacity = request.CalculateNewCapacity();
			auto [appended_blocks_count, block] = blocks.acquire_blocks(new_capacity *
																			sizeof(IndexPool::IndexType),
																		1);

			common_size += appended_blocks_count * blocks.get_block_size();
			if(appended_blocks_count == 0)
			{
				//no realloc
				auto old_blk = request.pool.GetBlock();
				request.MoveAndAppendToNew(buffer.memory.map_ptr, block.offset, block.size);
				blocks.release_block(old_blk);
			}

			return {common_size, UpdateRequestPrepared{request, block}};
		}
	}

	void UniformInstanceIndexAllocator::update_erase_no_acquire(const UpdateIndicesRequest &request, Buffer &buffer) noexcept
	{
		hrs::assert_true_debug(request.pool.GetSize() >= request.erase_start + request.erase_count,
							   "Erase indices are out of pool bounds!");

		request.ShrinkErase(buffer.memory.map_ptr);
	}

	std::pair<vk::DeviceSize, UpdateRequestPrepared>
	UniformInstanceIndexAllocator::update_erase_acquire(vk::DeviceSize common_size,
											 const UpdateIndicesRequest &request,
											 Buffer &buffer) noexcept
	{
		hrs::assert_true_debug(request.pool.GetSize() >= request.erase_start + request.erase_count,
							   "Erase indices are out of pool bounds!");

		IndexPool::IndexType free_size_after_erase = request.CalculateFreeSizeAfterErase();

		if(free_size_after_erase >= request.add_data.size())
		{
			//enough space
			request.ShrinkErase(buffer.memory.map_ptr);
			request.AppendData(buffer.memory.map_ptr);
			return {common_size, UpdateRequestPrepared{request, {}}};
		}
		else
		{
			//not enough space
			IndexPool::IndexType new_capacity = request.CalculateNewCapacity();
			auto [appended_blocks_count, block] = blocks.acquire_blocks(new_capacity *
																			sizeof(IndexPool::IndexType), 1);

			common_size += appended_blocks_count * blocks.get_block_size();
			if(appended_blocks_count == 0)
			{
				//no reallocation
				auto old_blk = request.pool.GetBlock();
				request.EraseAndAppendToNew(buffer.memory.map_ptr, block.offset, block.size);
				blocks.release_block(old_blk);
			}
			else
			{
				//reallocated
				request.ShrinkErase(buffer.memory.map_ptr);
			}

			return {common_size, UpdateRequestPrepared{request, block}};
		}
	}
};
