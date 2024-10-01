#include "DataBuffer.h"
#include "../Allocator/AllocateFromMany.hpp"
#include "../Context/Device.h"
#include <execution>

namespace FireLand
{
    DataBuffer::DataBuffer(Device* _parent_device,
                           std::uint32_t _rounding_item_count,
                           const hrs::mem_req<vk::DeviceSize>& _data_item_req,
                           const DataQueueReserves& reserves,
                           const std::function<NewPoolSizeCalculator>& _calc)
        : parent_device(_parent_device),
          rounding_item_count(_rounding_item_count),
          data_item_req(_data_item_req),
          queue(reserves),
          calc(_calc)
    {}

    DataBuffer::~DataBuffer()
    {
        Destroy();
    }

    DataBuffer::DataBuffer(DataBuffer&& db) noexcept
        : parent_device(db.parent_device),
          bounded_buffer_size(std::move(db.bounded_buffer_size)),
          rounding_item_count(db.rounding_item_count),
          free_blocks(std::move(db.free_blocks)),
          data_item_req(db.data_item_req),
          queue(std::move(db.queue)),
          calc(db.calc)
    {}

    DataBuffer& DataBuffer::operator=(DataBuffer&& db) noexcept
    {
        Destroy();

        parent_device = db.parent_device;
        bounded_buffer_size = std::move(db.bounded_buffer_size);
        rounding_item_count = db.rounding_item_count;
        free_blocks = std::move(db.free_blocks);
        data_item_req = db.data_item_req;
        queue = std::move(db.queue);
        calc = db.calc;

        return *this;
    }

    hrs::error DataBuffer::Recreate(std::uint32_t init_item_count)
    {
        Destroy();
        if(init_item_count == 0)
            return {};

        vk::DeviceSize buffer_size =
            hrs::round_up_size_to_alignment(init_item_count, rounding_item_count) *
            data_item_req.size;

        auto err = recreate_buffer(buffer_size);
        if(err)
            return err;

        free_blocks = hrs::unsized_free_block_chain<vk::DeviceSize>(buffer_size, 0);
        return {};
    }

    void DataBuffer::Destroy()
    {
        if(!IsCreated())
            return;

        destroy_buffer();
        free_blocks.clear();
        queue.Clear();
    }

    bool DataBuffer::IsCreated() const noexcept
    {
        return bounded_buffer_size.bounded_buffer.IsCreated();
    }

    void DataBuffer::NewRemoveOp(const DataRemoveOp& op)
    {
        queue.NewRemoveOp(op);
    }

    void DataBuffer::NewAddOp(DataAddOp op)
    {
        queue.NewAddOp(op);
    }

    void DataBuffer::NewUpdateOp(const DataUpdateOp& op)
    {
        queue.NewUpdateOp(op);
    }

    hrs::error DataBuffer::SyncAndWrite()
    {
        std::size_t free_count = std::numeric_limits<std::size_t>::max();
        if(queue.GetRemoves().empty())
            free_count = calculate_blocks_free_items();

        for(auto& remove: queue.GetRemoves())
        {
            const hrs::block<vk::DeviceSize> blk(data_item_req.size,
                                                 remove.index * data_item_req.size);
            free_blocks.release(blk);
        }

        if(!queue.HasWriteCommands())
        {
            queue.Clear();
            return {};
        }

        if(free_count < queue.GetAdds().size())
        {
            //realloc
            auto old_buffer = std::move(bounded_buffer_size);
            std::uint32_t append_count = queue.GetAdds().size() - free_count;
            std::uint32_t total_count = old_buffer.size / data_item_req.size + append_count;
            vk::DeviceSize buffer_size =
                hrs::round_up_size_to_alignment(total_count, rounding_item_count) *
                data_item_req.size;
            auto err = recreate_buffer(buffer_size);
            if(err)
                return err;

            free_blocks.increase_size(buffer_size - free_blocks.get_size());

            std::copy_n(std::execution::unseq,
                        old_buffer.bounded_buffer.GetBufferMapPtr(),
                        old_buffer.size,
                        bounded_buffer_size.bounded_buffer.GetBufferMapPtr());
        }

        for(auto& add: queue.GetAdds())
        {
            auto blk_pair = free_blocks.acquire(data_item_req);
            std::size_t offset = blk_pair.first.offset / data_item_req.size;
            *add.output_write = offset;
            std::copy_n(std::execution::unseq,
                        add.data.GetData(),
                        data_item_req.size,
                        bounded_buffer_size.bounded_buffer.GetBufferMapPtr() +
                            blk_pair.first.offset);
        }

        for(auto& upd_add: queue.GetUpdatedAdds())
        {
            *upd_add.add_op.output_write = upd_add.index;
            std::copy_n(std::execution::unseq,
                        upd_add.add_op.data.GetData(),
                        data_item_req.size,
                        bounded_buffer_size.bounded_buffer.GetBufferMapPtr() +
                            upd_add.index * data_item_req.size);
        }

        for(auto& upd: queue.GetUpdates())
        {
            std::copy_n(std::execution::unseq,
                        upd.data.GetData() + upd.data_block.offset,
                        upd.data_block.size,
                        bounded_buffer_size.bounded_buffer.GetBufferMapPtr() +
                            upd.index * data_item_req.size + upd.in_data_buffer_offset);
        }

        queue.Clear();
        return {};
    }

    Device* DataBuffer::GetParentDevice() noexcept
    {
        return parent_device;
    }

    const Device* DataBuffer::GetParentDevice() const noexcept
    {
        return parent_device;
    }

    vk::Buffer DataBuffer::GetHandle() const noexcept
    {
        return bounded_buffer_size.bounded_buffer.buffer;
    }

    std::uint32_t DataBuffer::GetBufferItemsSize() const noexcept
    {
        return bounded_buffer_size.size;
    }

    std::byte* DataBuffer::GetMappedPtr() noexcept
    {
        return bounded_buffer_size.bounded_buffer.GetBufferMapPtr();
    }

    const std::byte* DataBuffer::GetMappedPtr() const noexcept
    {
        return bounded_buffer_size.bounded_buffer.GetBufferMapPtr();
    }

    const hrs::mem_req<vk::DeviceSize>& DataBuffer::GetDataItemReq() const noexcept
    {
        return data_item_req;
    }

    hrs::error DataBuffer::recreate_buffer(vk::DeviceSize size)
    {
        destroy_buffer();
        constexpr static std::array variants = {
            MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eHostVisible,
                                  MemoryTypeSatisfyOp::Any,
                                  AllocationFlags::MapMemory),
            MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eHostVisible,
                                  MemoryTypeSatisfyOp::Any,
                                  hrs::flags(AllocationFlags::MapMemory) |
                                      AllocationFlags::AllowPlaceWithMixedResources),
        };

        const vk::BufferCreateInfo info({},
                                        size,
                                        vk::BufferUsageFlagBits::eStorageBuffer,
                                        vk::SharingMode::eExclusive);

        auto buffer_exp = AllocateFromMany(*parent_device->GetAllocator(),
                                           variants,
                                           info,
                                           data_item_req.alignment,
                                           calc);

        if(!buffer_exp)
            return buffer_exp.error();

        bounded_buffer_size = {std::move(buffer_exp.value()), size};
        return {};
    }

    void DataBuffer::destroy_buffer()
    {
        parent_device->GetAllocator()->Release(bounded_buffer_size.bounded_buffer,
                                               MemoryPoolOnEmptyPolicy::Free);
        bounded_buffer_size = {};
    }

    std::size_t DataBuffer::calculate_blocks_free_items() const noexcept
    {
        std::size_t count = 0;
        for(auto it = free_blocks.begin(); it != free_blocks.end(); it++)
            count += it->size / data_item_req.size;

        return count;
    }
};
