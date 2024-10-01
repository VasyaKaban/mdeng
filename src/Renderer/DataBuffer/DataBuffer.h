#pragma once

#include "../Allocator/BoundedResourceSize.hpp"
#include "../TransferChannel/TransferChannel.h"
#include "DataQueue.h"
#include "hrs/non_creatable.hpp"
#include "hrs/unsized_free_block_chain.hpp"

namespace FireLand
{
    class Device;

    class DataBuffer : public hrs::non_copyable
    {
    public:
        DataBuffer(Device* _parent_device = {},
                   std::uint32_t _rounding_item_count = {},
                   const hrs::mem_req<vk::DeviceSize>& _data_item_req = {},
                   const DataQueueReserves& reserves = {},
                   const std::function<NewPoolSizeCalculator>& _calc =
                       MemoryType::DefaultNewPoolSizeCalculator);
        ~DataBuffer();
        DataBuffer(DataBuffer&& db) noexcept;
        DataBuffer& operator=(DataBuffer&& db) noexcept;

        hrs::error Recreate(std::uint32_t init_item_count);

        void Destroy();
        bool IsCreated() const noexcept;

        void NewRemoveOp(const DataRemoveOp& op);
        void NewAddOp(DataAddOp op);
        void NewUpdateOp(const DataUpdateOp& op);

        hrs::error SyncAndWrite();

        Device* GetParentDevice() noexcept;
        const Device* GetParentDevice() const noexcept;
        vk::Buffer GetHandle() const noexcept;
        std::uint32_t GetBufferItemsSize() const noexcept;
        std::byte* GetMappedPtr() noexcept;
        const std::byte* GetMappedPtr() const noexcept;
        const hrs::mem_req<vk::DeviceSize>& GetDataItemReq() const noexcept;
    private:
        hrs::error recreate_buffer(vk::DeviceSize size);
        void destroy_buffer();
        std::size_t calculate_blocks_free_items() const noexcept;
    private:
        Device* parent_device;
        BoundedBufferSize bounded_buffer_size;
        std::uint32_t rounding_item_count;
        hrs::unsized_free_block_chain<vk::DeviceSize> free_blocks;
        hrs::mem_req<vk::DeviceSize> data_item_req;
        DataQueue queue;
        std::function<NewPoolSizeCalculator> calc;
    };
};
