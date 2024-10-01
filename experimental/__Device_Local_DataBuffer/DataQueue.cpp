#include "DataQueue.h"

namespace FireLand
{
    DataQueue::DataQueue(const DataQueueReserves& reserves)
    {
        removes.reserve(reserves.removes_reserve);
        adds.reserve(reserves.adds_reserve);
        updates.reserve(reserves.updates_reserve);
        updated_adds.reserve(reserves.updated_adds_reserve);
    }

    void DataQueue::NewRemoveOp(const DataRemoveOp& op)
    {
        if(!adds.empty())
            updated_adds.push_back({adds.back(), op.offset});
        else
            removes.push_back(op);
    }

    void DataQueue::NewAddOp(DataAddOp op)
    {
        if(!removes.empty())
        {
            updated_adds.push_back({op, removes.back().offset});
            removes.pop_back();
        }
        else
            adds.push_back(op);
    }

    void DataQueue::NewUpdateOp(const DataUpdateOp& op)
    {
        updates.push_back(op);
    }

    void DataQueue::Clear(const DataQueueReserves& reserves)
    {
        removes.clear();
        removes.reserve(reserves.removes_reserve);

        adds.clear();
        adds.reserve(reserves.adds_reserve);

        updates.clear();
        updates.reserve(reserves.updates_reserve);

        updated_adds.clear();
        updated_adds.reserve(reserves.updated_adds_reserve);
    }

    std::vector<DataRemoveOp>& DataQueue::GetRemoves() noexcept
    {
        return removes;
    }

    const std::vector<DataRemoveOp>& DataQueue::GetRemoves() const noexcept
    {
        return removes;
    }

    std::vector<DataAddOp>& DataQueue::GetAdds() noexcept
    {
        return adds;
    }

    const std::vector<DataAddOp>& DataQueue::GetAdds() const noexcept
    {
        return adds;
    }

    std::vector<DataUpdateOp>& DataQueue::GetUpdates() noexcept
    {
        return updates;
    }

    const std::vector<DataUpdateOp>& DataQueue::GetUpdates() const noexcept
    {
        return updates;
    }

    std::vector<DataUpdatedAddOp>& DataQueue::GetUpdatedAdds() noexcept
    {
        return updated_adds;
    }

    const std::vector<DataUpdatedAddOp>& DataQueue::GetUpdatedAdds() const noexcept
    {
        return updated_adds;
    }
};
