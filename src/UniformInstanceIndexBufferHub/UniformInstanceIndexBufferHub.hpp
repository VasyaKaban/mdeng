/**
 * @file
 *
 * Represenst the UniformInstanceIndexBufferHub class
 */

#pragma once

#include "UniformInstanceIndexAllocator.hpp"
#include "../Allocator/Allocator.hpp"
#include "../Allocator/ConditionalAllocate.hpp"

namespace FireLand
{
	/**
	 * @brief The UniformInstanceIndexBufferRequest class
	 *
	 * Used as remove/add/update request structure for UniformInstanceIndexBufferHub class
	 */
	struct UniformInstanceIndexBufferRequest
	{
		std::vector<RemoveIndicesRequest> remove_requests;///<remove requests
		std::vector<AcquireIndicesRequest> add_requests;///acquire requests
		std::vector<UpdateIndicesRequest> update_requests;///update requests
	};

	/**
	 * @brief The UniformInstanceIndexBufferHub class
	 *
	 * Used to take ownership for uniform index buffers and
	 * for handling remove/add/update requests by passing them to
	 * inner UniformInstanceIndexAllocator class object
	 */
	class UniformInstanceIndexBufferHub
	{
	private:
		/**
		 * @brief UniformInstanceIndexBufferHub
		 * @param _buffers created buffers
		 * @param _queue_family_index queue family index for buffer create info
		 * @param _usage buffer usage for create info
		 * @param _allocator allocator for buffer allocations
		 * @param _blocks_allocator inner UniformInstanceIndexAllocator for requests handling
		 */
		UniformInstanceIndexBufferHub(std::vector<Buffer> &&_buffers,
									  std::uint32_t _queue_family_index,
									  vk::BufferUsageFlags _usage,
									  Allocator *_allocator,
									  UniformInstanceIndexAllocator &&_blocks_allocator);
	public:

		/**
		 * @brief Create
		 * @param _allocator allocator for buffer allocations
		 * @param _queue_family_index queue family index for buffer create info
		 * @param _usage usage for create info
		 * @param initial_buffers_count initial count of index buffers
		 * @param initial_indices_count initial indices capacity of buffers
		 * @return created UniformInstanceIndexBufferHub or error result
		 */
		static hrs::expected<UniformInstanceIndexBufferHub, vk::Result>
		Create(Allocator *_allocator,
			   std::uint32_t _queue_family_index,
			   vk::BufferUsageFlags _usage,
			   std::size_t initial_buffers_count,
			   std::size_t initial_indices_count);

		~UniformInstanceIndexBufferHub();
		UniformInstanceIndexBufferHub(const UniformInstanceIndexBufferHub &) = delete;
		UniformInstanceIndexBufferHub(UniformInstanceIndexBufferHub &&uhub) noexcept;
		UniformInstanceIndexBufferHub & operator=(const UniformInstanceIndexBufferHub &) = delete;
		UniformInstanceIndexBufferHub & operator=(UniformInstanceIndexBufferHub &&uhub) noexcept;

		/**
		 * @brief GetBuffersCount
		 * @return count of already created buffers
		 */
		std::size_t GetBuffersCount() const noexcept;

		/**
		 * @brief operator []
		 * @param index index within inner buffers vector
		 * @return buffer at this index
		 * @warning This function doesn't check index bounds
		 */
		Buffer operator[](std::size_t index) noexcept;

		/**
		 * @brief GetTargetBuffer
		 * @return target buffer
		 */
		Buffer GetTargetBuffer() noexcept;

		/**
		 * @brief GetActualBuffer
		 * @return actual buffer(that has the newest data)
		 */
		Buffer GetActualBuffer() noexcept;

		/**
		 * @brief GetActualBufferSize
		 * @return actual buffer size
		 */
		vk::DeviceSize GetActualBufferSize() const noexcept;

		/**
		 * @brief Next
		 *
		 * Moves inner target buffer index.
		 */
		void Next() noexcept;

		/**
		 * @brief operator bool
		 *
		 * Checks whether object is created or not
		 * (simply checks if allocator is created and buffers vector isn't empty)
		 */
		constexpr explicit operator bool() const noexcept;

		/**
		 * @brief IsCreated
		 * @return bool if object is created and false otherwise
		 *
		 * Smae as opeartor bool
		 */
		constexpr bool IsCreated() const noexcept;

		/**
		 * @brief HandleRequests
		 * @param request remove/add/update requests object
		 * @return UniformInstanceIndexAllocatorResult on success and error result on failure
		 */
		hrs::expected<UniformInstanceIndexAllocatorResult, vk::Result>
		HandleRequests(const UniformInstanceIndexBufferRequest &request);

		/**
		 * @brief HandleRequests
		 * @param request remove/add/update requests object
		 * @return UniformInstanceIndexAllocatorResult on success and error result on failure
		 */
		hrs::expected<UniformInstanceIndexAllocatorResult, vk::Result>
		HandleRequests(UniformInstanceIndexBufferRequest &&request);

		/**
		 * @brief Reset
		 * @param initial_buffers_count initial count of index buffers
		 * @param initial_indices_count initial indices capacity of buffers
		 * @return vk::Result::Success on success and other value otherwise
		 */
		vk::Result Reset(std::size_t initial_buffers_count, std::size_t initial_indices_count);

	private:

		/**
		 * @brief sync_target
		 * @return synchonizes data in target buffer with actual one and
		 * makes the target buffer actual
		 */
		vk::Result sync_target();

		/**
		 * @brief allocate_buffer
		 * @param _allocator allocator for buffer allocations
		 * @param _queue_family_index queue family index for buffer create info
		 * @param _usage usage for create info
		 * @param size size of buffer
		 * @return allocated buffer or error result
		 */
		static hrs::expected<Buffer, vk::Result>
		allocate_buffer(Allocator *_allocator,
						std::uint32_t _queue_family_index,
						vk::BufferUsageFlags _usage,
						vk::DeviceSize size) noexcept;

		std::vector<Buffer> buffers;///<vector of index buffers
		std::uint32_t queue_family_index;///<queue family index for create info
		vk::BufferUsageFlags usage;///<usage flags for create info
		Allocator *allocator;///<allocator for buffer allocations
		UniformInstanceIndexAllocator blocks_allocator;///<index pools allocator for remove/add/update operations
		std::size_t target_buffer_index;///<index of target buffer
		decltype(std::ranges::ssize(buffers)) actual_buffer_index;///<index of actual buffer(that has the newest data)
	};

	UniformInstanceIndexBufferHub::UniformInstanceIndexBufferHub(std::vector<Buffer> &&_buffers,
																 std::uint32_t _queue_family_index,
																 vk::BufferUsageFlags _usage,
																 Allocator *_allocator,
																 UniformInstanceIndexAllocator &&_blocks_allocator)
		: allocator(_allocator),
		  queue_family_index(_queue_family_index),
		  usage(_usage),
		  buffers(std::move(_buffers)),
		  blocks_allocator(std::move(_blocks_allocator))
	{
		target_buffer_index = 0;
		actual_buffer_index = -1;
	}

	hrs::expected<UniformInstanceIndexBufferHub, vk::Result>
	UniformInstanceIndexBufferHub::Create(Allocator *_allocator,
										  std::uint32_t _queue_family_index,
										  vk::BufferUsageFlags _usage,
										  std::size_t initial_buffers_count,
										  std::size_t initial_indices_count)
	{
		hrs::assert_true_debug(_allocator->IsCreated(), "Allocator isn't created yet!");
		hrs::assert_true_debug(initial_buffers_count > 0,
							   "The Initial count of buffers must be greater than zero!");

		UniformInstanceIndexAllocator _blocks_allocator(initial_indices_count);
		std::vector<Buffer> _buffers(initial_buffers_count);


		if(initial_buffers_count != 0)
		{
			for(std::size_t i = 0; i < _buffers.size(); i++)
			{
				auto alloc_result = allocate_buffer(_allocator,
													_queue_family_index,
													_usage,
													_blocks_allocator.GetSize());
				if(!alloc_result.has_value())
				{
					for(std::size_t j = 0; j < i; j++)
						_buffers[i].Destroy(_allocator->GetDevice());

					return {vk::Result::eErrorOutOfHostMemory};
				}

				_buffers[i] = std::move(alloc_result.value());
			}
		}

		return UniformInstanceIndexBufferHub(std::move(_buffers),
									 _queue_family_index,
									 _usage,
									 _allocator,
									 std::move(_blocks_allocator));
	}

	UniformInstanceIndexBufferHub::~UniformInstanceIndexBufferHub()
	{
		if(IsCreated())
		{
			for(auto &buf : buffers)
				buf.Destroy(allocator->GetDevice());

			blocks_allocator.Clear(0);
		}
	}

	UniformInstanceIndexBufferHub::UniformInstanceIndexBufferHub(UniformInstanceIndexBufferHub &&uhub) noexcept
		: allocator(uhub.allocator),
		  queue_family_index(uhub.queue_family_index),
		  usage(uhub.usage),
		  buffers(std::move(uhub.buffers)),
		  blocks_allocator(std::move(uhub.blocks_allocator)),
		  target_buffer_index(uhub.target_buffer_index),
		  actual_buffer_index(uhub.actual_buffer_index) {}

	UniformInstanceIndexBufferHub &
	UniformInstanceIndexBufferHub::operator=(UniformInstanceIndexBufferHub &&uhub) noexcept
	{
		this->~UniformInstanceIndexBufferHub();

		allocator = uhub.allocator;
		queue_family_index = uhub.queue_family_index;
		usage = uhub.usage;
		buffers = std::move(uhub.buffers);
		blocks_allocator = std::move(uhub.blocks_allocator);
		target_buffer_index = uhub.target_buffer_index;
		actual_buffer_index = uhub.actual_buffer_index;
		return *this;
	}

	std::size_t UniformInstanceIndexBufferHub::GetBuffersCount() const noexcept
	{
		return buffers.size();
	}

	Buffer UniformInstanceIndexBufferHub::operator[](std::size_t index) noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");
		return buffers[index];
	}

	Buffer UniformInstanceIndexBufferHub::GetTargetBuffer() noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");
		return buffers[target_buffer_index];
	}

	Buffer UniformInstanceIndexBufferHub::GetActualBuffer() noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");
		if(actual_buffer_index == -1)
			return buffers[0];

		return buffers[actual_buffer_index];
	}

	vk::DeviceSize UniformInstanceIndexBufferHub::GetActualBufferSize() const noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");
		if(actual_buffer_index == -1)
			return buffers[0].memory.size;

		return buffers[actual_buffer_index].memory.size;
	}

	void UniformInstanceIndexBufferHub::Next() noexcept
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");
		target_buffer_index = (target_buffer_index + 1) % buffers.size();
	}

	constexpr UniformInstanceIndexBufferHub::operator bool() const noexcept
	{
		return allocator != nullptr && !buffers.empty();
	}

	constexpr bool UniformInstanceIndexBufferHub::IsCreated() const noexcept
	{
		return static_cast<bool>(*this);
	}

	hrs::expected<UniformInstanceIndexAllocatorResult, vk::Result>
	UniformInstanceIndexBufferHub::HandleRequests(const UniformInstanceIndexBufferRequest &request)
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");

		vk::Result sync_result = sync_target();
		if(sync_result != vk::Result::eSuccess)
			return sync_result;

		auto realloc_func = [this](vk::DeviceSize size)
		{
			return allocate_buffer(allocator,
								   queue_family_index,
								   usage,
								   size);
		};


		UniformInstanceIndexAllocatorRequest<decltype(realloc_func)> block_alloc_request
		{
			.buffer = buffers[target_buffer_index],
			.alloc_func = realloc_func,
			.device = allocator->GetDevice(),
			.remove_requests = request.remove_requests,
			.add_requests = request.add_requests,
			.update_requests = request.update_requests
		};

		return blocks_allocator.HandleRequests(block_alloc_request);
	}

	hrs::expected<UniformInstanceIndexAllocatorResult, vk::Result>
	UniformInstanceIndexBufferHub::HandleRequests(UniformInstanceIndexBufferRequest &&request)
	{
		hrs::assert_true_debug(IsCreated(), "Object isn't created yet!");

		vk::Result sync_result = sync_target();
		if(sync_result != vk::Result::eSuccess)
			return sync_result;

		auto realloc_func = [this](vk::DeviceSize size)
		{
			return allocate_buffer(allocator,
								   queue_family_index,
								   usage,
								   size);
		};


		UniformInstanceIndexAllocatorRequest<decltype(realloc_func)> block_alloc_request
		{
			.buffer = buffers[target_buffer_index],
			.alloc_func = realloc_func,
			.device = allocator->GetDevice(),
			.remove_requests = std::move(request.remove_requests),
			.add_requests = std::move(request.add_requests),
			.update_requests = std::move(request.update_requests)
		};

		return blocks_allocator.HandleRequests(block_alloc_request);
	}

	vk::Result UniformInstanceIndexBufferHub::Reset(std::size_t initial_buffers_count,
													std::size_t initial_indices_count)
	{
		hrs::assert_true_debug(initial_buffers_count > 0,
							   "The Initial count of buffers must be greater than zero!");

		this->~UniformInstanceIndexBufferHub();
		blocks_allocator.Clear(initial_indices_count);
		for(std::size_t i = 0; i < initial_buffers_count; i++)
		{
			auto alloc_result = allocate_buffer(allocator,
												queue_family_index,
												usage,
												blocks_allocator.GetSize());
			if(!alloc_result)
			{
				this->~UniformInstanceIndexBufferHub();
				return alloc_result.error();
			}

			buffers[i] = std::move(alloc_result.value());
		}

		actual_buffer_index = -1;
		target_buffer_index = 0;

		return vk::Result::eSuccess;
	}

	vk::Result UniformInstanceIndexBufferHub::sync_target()
	{
		if(actual_buffer_index == -1 || target_buffer_index == actual_buffer_index)
			return vk::Result::eSuccess;

		Buffer &target_buffer = buffers[target_buffer_index];
		Buffer &actual_buffer = buffers[actual_buffer_index];

		hrs::assert_true_debug(target_buffer.memory.size > actual_buffer.memory.size,
							   "Strange buffer behaviour! Target size is bigger than actual one!");

		if(target_buffer.memory.size < actual_buffer.memory.size)
		{
			//not wnough space -> realloc
			target_buffer.Destroy(allocator->GetDevice());
			auto alloc_result = allocate_buffer(allocator,
												queue_family_index,
												usage,
												actual_buffer.memory.size);

			if(!alloc_result)
				return alloc_result.error();

			target_buffer = std::move(alloc_result.value());
		}

		std::memcpy(target_buffer.memory.map_ptr,
					actual_buffer.memory.map_ptr,
					actual_buffer.memory.size);

		actual_buffer_index = target_buffer_index;

		return vk::Result::eSuccess;
	}

	hrs::expected<Buffer, vk::Result>
	UniformInstanceIndexBufferHub::allocate_buffer(Allocator *_allocator,
												   std::uint32_t _queue_family_index,
												   vk::BufferUsageFlags _usage,
												   vk::DeviceSize size) noexcept
	{
		vk::BufferCreateInfo buffer_info({},
										 size,
										 _usage | vk::BufferUsageFlagBits::eStorageBuffer,
										 vk::SharingMode::eExclusive,
										 1,
										 &_queue_family_index);

		constexpr static auto buffer_conds = std::array
		{
			AllocateConditionalInfo{DesiredType::Only, vk::MemoryPropertyFlagBits::eHostVisible, true},
			AllocateConditionalInfo{DesiredType::Any, vk::MemoryPropertyFlagBits::eHostVisible, true}
		};

		auto alloc_result = AllocateBuffer(*_allocator, buffer_conds, buffer_info);
		if(alloc_result)
			return alloc_result.value().first;

		return {vk::Result::eErrorOutOfHostMemory};
	}
};
