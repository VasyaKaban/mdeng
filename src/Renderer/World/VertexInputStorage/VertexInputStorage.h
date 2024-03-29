#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../../../hrs/expected.hpp"

namespace FireLand
{
	struct VertexBufferBlock
	{
		std::list<VertexBuffer>::iterator buffer;
		hrs::block<vk::DeviceSize> block;

		VertexBufferBlock(std::list<VertexBuffer>::iterator _buffer = {},
						  const hrs::block<vk::DeviceSize> &_block = {}) noexcept
			: buffer(_buffer),
			  block(_block) {}
	};

	struct IndexBufferBlock
	{
		std::list<IndexBuffer>::iterator buffer;
		hrs::block<vk::DeviceSize> block;

		IndexBufferBlock(std::list<IndexBuffer>::iterator _buffer = {},
						 const hrs::block<vk::DeviceSize> &_block = {}) noexcept
			: buffer(_buffer),
			  block(_block) {}
	};

	struct VertexDataDescription
	{
		vk::DeviceSize data_size;
		vk::DeviceSize data_alignment;

		VertexDataDescription(vk::DeviceSize _data_size, vk::DeviceSize _data_alignment) noexcept
			: data_size(_data_size),
			  data_alignment(_data_alignment) {}
	};

	struct IndexDataDescription
	{
		std::uint32_t count;

		IndexDataDescription(std::uint32_t _count) noexcept
			: count(_count) {}
	};

	class VertexInputStorage : public hrs::non_copyable
	{
	public:
		VertexInputStorage(Device *_parent_device,
						   vk::DeviceSize _new_allocation_round_up_size,
						   std::uint32_t _new_allocation_round_up_count) noexcept;
		~VertexInputStorage();
		VertexInputStorage(VertexInputStorage &&vis) noexcept;
		VertexInputStorage & operator=(VertexInputStorage &&vis) noexcept;

		void Destroy() noexcept;

		hrs::expected<VertexBufferBlock, hrs::unexpected_result>
		Allocate(VertexDataDescription vertex_data_description);//allocates block for vertex data

		hrs::expected<IndexBufferBlock, hrs::unexpected_result>
		Allocate(IndexDataDescription index_data_description);//allocates block for indices data

		void Release(const VertexBufferBlock &vbb);
		void Release(const IndexBufferBlock &ibb);

		void Free(std::list<VertexBuffer>::iterator buffer);
		void Free(std::list<IndexBuffer>::iterator buffer);

	private:
		Device *parent_device;
		std::list<VertexBuffer> vertex_buffers;
		vk::DeviceSize new_allocation_round_up_size;
		std::list<IndexBuffer> index_buffers;
		std::uint32_t new_allocation_round_up_count;
	};
};
