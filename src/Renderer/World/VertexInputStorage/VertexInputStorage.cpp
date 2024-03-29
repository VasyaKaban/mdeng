#include "VertexInputStorage.h"
#include "../../Context/Device.h"

namespace FireLand
{
	VertexInputStorage::VertexInputStorage(Device *_parent_device,
										   vk::DeviceSize _new_allocation_round_up_size,
										   std::uint32_t _new_allocation_round_up_count) noexcept
		: parent_device(_parent_device),
		  new_allocation_round_up_size(_new_allocation_round_up_size),
		  new_allocation_round_up_count(_new_allocation_round_up_count)
	{
		hrs::assert_true_debug(_parent_device, "Parent device pointer points to null!");
		hrs::assert_true_debug(_parent_device->GetHandle(), "Parent device isn't created yet!");
	}

	VertexInputStorage::~VertexInputStorage()
	{
		Destroy();
	}

	VertexInputStorage::VertexInputStorage(VertexInputStorage &&vis) noexcept
		: parent_device(vis.parent_device),
		  new_allocation_round_up_size(vis.new_allocation_round_up_size),
		  new_allocation_round_up_count(vis.new_allocation_round_up_count),
		  vertex_buffers(std::move(vis.vertex_buffers)),
		  index_buffers(std::move(vis.index_buffers)) {}

	VertexInputStorage & VertexInputStorage::operator=(VertexInputStorage &&vis) noexcept
	{
		Destroy();

		parent_device = vis.parent_device;
		new_allocation_round_up_size = vis.new_allocation_round_up_size;
		new_allocation_round_up_count = vis.new_allocation_round_up_count;
		vertex_buffers = std::move(vis.vertex_buffers);
		index_buffers = std::move(vis.index_buffers);

		return *this;
	}

	void VertexInputStorage::Destroy() noexcept
	{
		vertex_buffers.clear();
		index_buffers.clear();
	}

	hrs::expected<VertexBufferBlock, hrs::unexpected_result>
	VertexInputStorage::Allocate(VertexDataDescription vertex_data_description)
	{
		for(auto it = vertex_buffers.begin(); it != vertex_buffers.end(); it++)
		{
			auto blk_opt = it->Acquire(vertex_data_description.data_size, vertex_data_description.data_alignment);
			if(blk_opt)
				return VertexBufferBlock(it, blk_opt.value());
		}

		VertexBuffer allocated_buffer(parent_device);
		vk::DeviceSize allocation_size = hrs::round_up_size_to_alignment(vertex_data_description.data_size,
																		 new_allocation_round_up_size);
		auto unexp_res = allocated_buffer.Recreate(allocation_size);
		if(unexp_res)
			return unexp_res;

		auto blk_opt = allocated_buffer.Acquire(vertex_data_description.data_size, vertex_data_description.data_alignment);
		vertex_buffers.push_back(std::move(allocated_buffer));
		return VertexBufferBlock(std::prev(vertex_buffers.end()), blk_opt.value());
	}

	hrs::expected<IndexBufferBlock, hrs::unexpected_result>
	VertexInputStorage::Allocate(IndexDataDescription index_data_description)
	{
		for(auto it = index_buffers.begin(); it != index_buffers.end(); it++)
		{
			auto blk_opt = it->Acquire(index_data_description.count * sizeof(std::uint32_t), sizeof(std::uint32_t));
			if(blk_opt)
				return IndexBufferBlock(it, blk_opt.value());
		}

		IndexBuffer allocated_buffer(parent_device);
		vk::DeviceSize allocation_size =
			hrs::round_up_size_to_alignment(index_data_description.count, new_allocation_round_up_count) *
										 sizeof(std::uint32_t);

		auto unexp_res = allocated_buffer.Recreate(allocation_size);
		if(unexp_res)
			return unexp_res;

		auto blk_opt = allocated_buffer.Acquire(index_data_description.count * sizeof(std::uint32_t),
												sizeof(std::uint32_t));
		index_buffers.push_back(std::move(allocated_buffer));
		return IndexBufferBlock(std::prev(index_buffers.end()), blk_opt.value());
	}

	void VertexInputStorage::Release(const VertexBufferBlock &vbb)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(vertex_buffers, vbb.buffer),
							   "Passes vertex buffer is not a part of this storage!");

		vbb.buffer->Release(vbb.block);
	}

	void VertexInputStorage::Release(const IndexBufferBlock &ibb)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(index_buffers, ibb.buffer),
							   "Passes vertex buffer is not a part of this storage!");

		ibb.buffer->Release(ibb.block);
	}

	void VertexInputStorage::Free(std::list<VertexBuffer>::iterator buffer)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(vertex_buffers, buffer),
							   "Buffer is not a part of these vertex buffers!");

		vertex_buffers.erase(buffer);
	}

	void VertexInputStorage::Free(std::list<IndexBuffer>::iterator buffer)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(index_buffers, buffer),
							   "Buffer is not a part of these index buffers!");

		index_buffers.erase(buffer);
	}
};
