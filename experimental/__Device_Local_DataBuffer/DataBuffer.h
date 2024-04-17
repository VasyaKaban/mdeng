#pragma once

#include "../../hrs/non_creatable.hpp"
#include "../../hrs/unsized_free_block_chain.hpp"
#include "../../hrs/unexpected_result.hpp"
#include "../Allocator/BoundedResourceSize.hpp"
#include "DataQueue.h"
#include "../TransferChannel/TransferChannel.h"

namespace FireLand
{
	class Device;

	class DataBuffer : public hrs::non_copyable
	{		
	public:
		DataBuffer(Device *_parent_device = {},
				   const hrs::mem_req<vk::DeviceSize> &_data_item_req = {},
				   const DataQueueReserves &reserves = {});
		~DataBuffer();
		DataBuffer(DataBuffer &&db) noexcept;
		DataBuffer & operator=(DataBuffer &&db) noexcept;

		hrs::unexpected_result Recreate(std::uint32_t init_item_count,
										const std::function<NewPoolSizeCalculator> &calc,
										const DataQueueReserves &reserves);

		void Destroy();
		bool IsCreated() const noexcept;

		void NewRemoveOp(const DataRemoveOp &op);
		void NewAddOp(DataAddOp op);
		void NewUpdateOp(const DataUpdateOp &op);

		hrs::unexpected_result SyncAndWrite(std::uint32_t rounding_item_count,
											TransferChannel &transfer,
											const std::function<NewPoolSizeCalculator> &calc);

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		vk::Buffer GetHandle() const noexcept;
		std::uint32_t GetBufferItemsSize() const noexcept;
		std::byte * GetMappedPtr() noexcept;
		const std::byte * GetMappedPtr() const noexcept;
		const hrs::mem_req<vk::DeviceSize> & GetDataItemReq() const noexcept;

	private:

		hrs::unexpected_result recreate_buffer(vk::DeviceSize size,
											   const std::function<NewPoolSizeCalculator> &calc);
		void destroy_buffer();

		std::size_t calculate_blocks_free_items() const noexcept;

		Device *parent_device;
		BoundedBufferSize bounded_buffer_size;
		hrs::unsized_free_block_chain<vk::DeviceSize> free_blocks;
		hrs::mem_req<vk::DeviceSize> data_item_req;
		DataQueue queue;
	};
};
