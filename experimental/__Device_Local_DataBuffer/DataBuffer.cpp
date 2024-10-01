#include "DataBuffer.h"
#include "../Allocator/AllocateFromMany.hpp"
#include "../Context/Device.h"

namespace FireLand
{
    DataBuffer::DataBuffer(Device* _parent_device,
                           const hrs::mem_req<vk::DeviceSize>& _data_item_req,
                           const DataQueueReserves& reserves)
        : parent_device(_parent_device),
          data_item_req(_data_item_req),
          queue(reserves)
    {}

    DataBuffer::~DataBuffer()
    {
        Destroy();
    }

    DataBuffer::DataBuffer(DataBuffer&& db) noexcept
        : parent_device(db.parent_device),
          bounded_buffer_size(std::move(db.bounded_buffer_size)),
          free_blocks(std::move(db.free_blocks)),
          data_item_req(db.data_item_req),
          queue(std::move(db.queue))
    {}

    DataBuffer& DataBuffer::operator=(DataBuffer&& db) noexcept
    {
        Destroy();

        parent_device = db.parent_device;
        bounded_buffer_size = std::move(db.bounded_buffer_size);
        free_blocks = std::move(db.free_blocks);
        data_item_req = db.data_item_req;
        queue = std::move(db.queue);

        return *this;
    }

    hrs::unexpected_result DataBuffer::Recreate(std::uint32_t init_item_count,
                                                const std::function<NewPoolSizeCalculator>& calc,
                                                const DataQueueReserves& reserves)
    {
        Destroy();
        queue.Clear(reserves);
        if(init_item_count == 0)
            return {};

        auto unexp_res = recreate_buffer(data_item_req.size * init_item_count, calc);
        if(unexp_res)
            return unexp_res;

        return {};
    }

    void DataBuffer::Destroy()
    {
        if(!IsCreated())
            return;

        destroy_buffer();
        free_blocks.clear();
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

    hrs::unexpected_result
    DataBuffer::SyncAndWrite(std::uint32_t rounding_item_count,
                             TransferChannel& transfer,
                             const std::function<NewPoolSizeCalculator>& calc)
    {
#error CHECK THIS!!!
#error LOL! WHY WE USE TRANSFER IF DATABUFFER IS HOST_VISIBLE ITSELF?!
        std::size_t free_count = std::numeric_limits<std::size_t>::max();
        if(queue.GetRemoves().empty())
            free_count = calculate_blocks_free_items();

        for(auto& remove: queue.GetRemoves())
        {
            const hrs::block<vk::DeviceSize> blk(data_item_req.size,
                                                 remove.offset * data_item_req.size);
            free_blocks.release(blk);
        }

        if(free_count < queue.GetAdds().size())
        {
            //realloc
            auto old_buffer = std::move(bounded_buffer_size);
            vk::DeviceSize append_size = (queue.GetAdds().size() - free_count) * data_item_req.size;
            vk::DeviceSize new_size =
                hrs::round_up_size_to_alignment(old_buffer.size + append_size, data_item_req.size);
            auto unexp_res = recreate_buffer(new_size, calc);
            if(unexp_res)
                return unexp_res;

            free_blocks.increase_size(new_size - free_blocks.get_size());

            const TransferBufferOpRegion region({old_buffer.size, 0}, 0, 0);
            Data old_buffer_data(old_buffer.bounded_buffer.GetBufferMapPtr());
            auto index_exp = transfer.CopyBuffer(bounded_buffer_size.bounded_buffer.buffer,
                                                 {&old_buffer_data, 1},
                                                 {&region, 1},
                                                 1);

            if(!index_exp)
                return index_exp.error();

            const vk::BufferMemoryBarrier barrier(vk::AccessFlagBits::eHostWrite,
                                                  vk::AccessFlagBits::eHostWrite |
                                                      vk::AccessFlagBits::eMemoryRead,
                                                  VK_QUEUE_FAMILY_IGNORED,
                                                  VK_QUEUE_FAMILY_IGNORED,
                                                  bounded_buffer_size.bounded_buffer.buffer,
                                                  0,
                                                  old_buffer.size);

            transfer.EmbedBarrier(vk::PipelineStageFlagBits::eTransfer,
                                  vk::PipelineStageFlagBits::eTransfer,
                                  {},
                                  {},
                                  {&barrier, 1},
                                  {});

            transfer.AddWaitFunction(
                [old_buf = std::move(old_buffer), allocator = parent_device->GetAllocator()]()
                {
                    allocator->Release(old_buf.bounded_buffer, MemoryPoolOnEmptyPolicy::Free);
                });
        }

        std::vector<TransferBufferOpRegion> regions;
        std::vector<Data> datas;
        regions.reserve(queue.GetAdds().size() + queue.GetUpdates().size() +
                        queue.GetUpdatedAdds().size());
        datas.reserve(regions.capacity());

        for(auto& add: queue.GetAdds())
        {
            datas.push_back(add.data);
            auto blk_pair = free_blocks.acquire(data_item_req);
            std::size_t offset = blk_pair.first.offset / data_item_req.size;
            *add.output_write = offset;
            const TransferBufferOpRegion region({data_item_req.size, 0},
                                                blk_pair.first.offset,
                                                datas.size() - 1);
            regions.push_back(region);
        }

        for(auto& upd_add: queue.GetUpdatedAdds())
        {
            datas.push_back(upd_add.add_op.data);
            const TransferBufferOpRegion region({data_item_req.size, 0},
                                                upd_add.offset * data_item_req.size,
                                                datas.size() - 1);

            regions.push_back(region);
            *upd_add.add_op.output_write = upd_add.offset;
        }

        for(auto& upd: queue.GetUpdates())
        {
            datas.push_back(upd.data);
            const TransferBufferOpRegion region(upd.data_block,
                                                upd.offset * data_item_req.size +
                                                    upd.in_data_buffer_offset,
                                                datas.size() - 1);

            regions.push_back(region);
        }

        auto unexp_exp =
            transfer.CopyBuffer(bounded_buffer_size.bounded_buffer.buffer, datas, regions, 1);

        if(!unexp_exp)
            return unexp_exp.error();

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

    hrs::unexpected_result
    DataBuffer::recreate_buffer(vk::DeviceSize size,
                                const std::function<NewPoolSizeCalculator>& calc)
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
            return AllocateFromManyErrorToUnexpectedResult(buffer_exp.error());

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
