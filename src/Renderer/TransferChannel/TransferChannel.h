#pragma once

#include "../Allocator/BoundedResourceSize.hpp"
#include "../../hrs/non_creatable.hpp"
#include "TransferBufferOp.hpp"
#include "TransferImageOp.hpp"

namespace FireLand
{
	class Device;

	struct BoundedBufferSizeFillness
	{
		BoundedBufferSize bounded_buffer_size;
		vk::DeviceSize fillness;

		BoundedBufferSizeFillness(BoundedBufferSize &&_bounded_buffer_size = {},
								  vk::DeviceSize _fillness = {}) noexcept
			: bounded_buffer_size(std::move(_bounded_buffer_size)),
			  fillness(_fillness){}

		~BoundedBufferSizeFillness() = default;
		BoundedBufferSizeFillness(BoundedBufferSizeFillness &&) = default;
		BoundedBufferSizeFillness & operator=(BoundedBufferSizeFillness &&) = default;

		bool IsEnoughSpace(vk::DeviceSize size) const noexcept
		{
			return size <= (bounded_buffer_size.size - fillness);
		}
	};

	class TransferChannel : public hrs::non_copyable
	{
		void init(BoundedBufferSizeFillness &&buffer,
				  vk::Fence _wait_fence,
				  vk::Queue _transfer_queue,
				  vk::CommandBuffer _command_buffer,
				  const std::function<NewPoolSizeCalculator> &_calc,
				  vk::DeviceSize _rounding_size,
				  vk::DeviceSize _buffer_size_power,
				  vk::DeviceSize _buffer_alignment) noexcept;
	public:
		TransferChannel(Device *_parent_device) noexcept;
		~TransferChannel();
		TransferChannel(TransferChannel &&tc) noexcept;
		TransferChannel & operator=(TransferChannel &&tc) noexcept;

		hrs::error Recreate(vk::Queue _transfer_queue,
							vk::CommandBuffer _command_buffer,
							vk::DeviceSize _rounding_size,
							std::size_t _buffer_size_power,
							vk::DeviceSize _buffer_alignment,
							const std::function<NewPoolSizeCalculator> &_calc);

		vk::CommandBuffer GetCommandBuffer() const noexcept;

		void Destroy() noexcept;
		bool IsCreated() const noexcept;

		vk::Result Flush(std::span<const vk::SubmitInfo> submits) noexcept;

		vk::Result GetWaitFenceStatus() const noexcept;
		vk::Result PlainWaitFence(std::uint64_t timeout =
								  std::numeric_limits<std::uint64_t>::max()) const noexcept;
		vk::Result WaitFence(std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;

		hrs::expected<std::size_t, hrs::error>
		CopyBuffer(vk::Buffer dst_buffer,
				   std::span<const Data> datas,
				   std::span<const TransferBufferOpRegion> regions);

		//do not care about minImageTransferGranularity because we want to transfer the whole mip level!
		hrs::expected<std::size_t, hrs::error>
		CopyImageSubresource(vk::Image dst_image,
							 vk::ImageLayout image_layout,
							 vk::DeviceSize block_size,
							 std::span<const Data> datas,
							 std::span<const TransferImageOpRegion> regions);

		vk::Result EmbedBarrier(vk::PipelineStageFlags src_stages,
								vk::PipelineStageFlags dst_stages,
								vk::DependencyFlags dependency,
								std::span<const vk::MemoryBarrier> memory_barriers,
								std::span<const vk::BufferMemoryBarrier> buffer_memory_barriers,
								std::span<const vk::ImageMemoryBarrier> image_memory_barriers);


		template<std::invocable<vk::CommandBuffer, vk::Buffer, vk::DeviceSize> F>
		hrs::expected<std::size_t, hrs::error>
		EmbedOperation(F &&func, vk::DeviceSize required_size);

		template<std::invocable F>
		void AddWaitFunction(F &&func);

		bool IsWriteStarted() const noexcept;

		Device * GetParentDevice () noexcept;
		const Device * GetParentDevice () const noexcept;
		vk::Queue GetTransferQueue() const noexcept;
		const std::function<NewPoolSizeCalculator> & GetNewPoolSizeCalculator() const noexcept;
		vk::DeviceSize GetRoundingSize() const noexcept;
		vk::DeviceSize GetBufferAlignment() const noexcept;
		vk::Fence GetFence() const noexcept;

		const BoundedBufferSize & GetBuffer(std::size_t index) const noexcept;

		hrs::error FlattenBuffers(std::size_t lower_buffer_size_power,
								  std::size_t higher_buffer_size_power);

	private:
		hrs::expected<BoundedBufferSize, hrs::error>
		allocate_buffer(const hrs::mem_req<vk::DeviceSize> &req,
						const std::function<NewPoolSizeCalculator> &_calc);

		vk::Result start_write() noexcept;


		void copy_buffer(vk::Buffer dst_buffer,
						 std::span<const Data> datas,
						 std::span<const TransferBufferOpRegion> regions,
						 BoundedBufferSizeFillness &buffer);

		void copy_image(vk::Image dst_image,
						vk::ImageLayout image_layout,
						vk::DeviceSize block_size,
						std::span<const Data> datas,
						std::span<const TransferImageOpRegion> regions,
						BoundedBufferSizeFillness &buffer);

		vk::DeviceSize calculate_region_size(const vk::Extent3D &extent,
											 vk::DeviceSize block_size) const noexcept;

		void free_buffers() noexcept;

		hrs::error push_new_buffer(vk::DeviceSize common_size);

	private:
		Device *parent_device;
		std::vector<BoundedBufferSizeFillness> buffers;
		vk::Fence wait_fence;
		std::vector<std::function<void ()>> pending_wait_functions;
		vk::Queue transfer_queue;
		std::function<NewPoolSizeCalculator> calc;
		vk::DeviceSize rounding_size;
		vk::DeviceSize buffer_alignment;
		vk::DeviceSize buffer_size_power;
		bool is_in_write;
		vk::CommandBuffer command_buffer;
	};

	template<std::invocable<vk::CommandBuffer, vk::Buffer, vk::DeviceSize> F>
	hrs::expected<std::size_t, hrs::error>
	TransferChannel::EmbedOperation(F &&func,
									vk::DeviceSize required_size)
	{
		constexpr auto call_func_and_fill =
		[&func, required_size, this](BoundedBufferSizeFillness &buffer)
		{
			std::forward<F>(func)(command_buffer,
								  buffer.bounded_buffer_size.bounded_buffer.buffer,
								  buffer.fillness);

			buffer.fillness += required_size;
		};


		if(required_size == 0)
			std::forward<F>(func)(command_buffer, VK_NULL_HANDLE, 0);
		else
		{
			for(std::size_t i = 0; i < buffers.size(); i++)
			{
				auto &buffer = buffers[i];
				if(buffer.IsEnoughSpace(required_size))
				{
					if(!IsWriteStarted())
					{
						vk::Result res = start_write();
						if(res != vk::Result::eSuccess)
							return hrs::error(res);
					}

					call_func_and_fill(buffer);
					return i;
				}
			}

			auto unexp_res = push_new_buffer(required_size);
			if(unexp_res)
				return unexp_res;

			call_func_and_fill(buffers.back());
			return buffers.size() - 1;
		}
	}

	template<std::invocable F>
	void TransferChannel::AddWaitFunction(F &&func)
	{
		pending_wait_functions.push_back(std::forward<F>(func));
	}
};
