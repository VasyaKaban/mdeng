#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "hrs/non_creatable.hpp"
#include <cstdint>
#include <vector>

namespace FireLand
{
    class DataIndexStorage;

    class IndexPool
    {
    public:
        IndexPool(DataIndexStorage* _parent_storage,
                  std::uint32_t _init_size_power = {},
                  std::uint32_t _rounding_size = {});
        ~IndexPool() = default;
        IndexPool(IndexPool&& ip) noexcept;
        IndexPool& operator=(IndexPool&& ip) noexcept;

        void NewAddOp(std::uint32_t data_index, std::uint32_t* subscriber_ptr);
        void NewRemoveOp(std::uint32_t index) noexcept;

        bool IsSyncNeeded() const noexcept;
        bool HasData() const noexcept;
        void Sync();

        DataIndexStorage* GetParentStorage() noexcept;
        const DataIndexStorage* GetParentStorage() const noexcept;
        std::uint32_t GetSize() const noexcept;
        std::uint32_t GetPreSyncSize() const noexcept;
        std::uint32_t GetFillness() const noexcept;
        vk::DeviceSize GetPoolOffset() const noexcept;
        const std::vector<std::uint32_t>& GetVirtualIndices() const noexcept;

        void UpdateSize() noexcept;
        void UpdateOffset(vk::DeviceSize new_pool_offset) noexcept;
    private:
        DataIndexStorage* parent_storage;
        vk::DeviceSize pool_offset;
        std::uint32_t size;
        std::uint32_t pre_sync_size;
        std::uint32_t fillness;
        std::uint32_t rounding_size;
        bool is_sync_needed;
        std::vector<std::uint32_t> virtual_indices;
        std::vector<std::uint32_t*> index_subscribers;
    };
};
