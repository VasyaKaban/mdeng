#pragma once

#include "../Allocator/BoundedResourceSize.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/unsized_free_block_chain.hpp"
#include <vector>

namespace FireLand
{
    class Device;
    class IndexPool;

    /*
	 remove -> free
	 add -> acquire and copy(if has data)
	 update -> copy
	 realloc -> acquire + copy
	 */

    struct AddPoolOp
    {
        IndexPool* pool;
        bool copy_data;

        constexpr AddPoolOp(IndexPool* _pool = {}, bool _copy_data = {}) noexcept
            : pool(_pool),
              copy_data(_copy_data)
        {}
    };

    class DataIndexStorage : public hrs::non_copyable
    {
        void init(std::vector<BoundedBufferSize>&& _index_buffers,
                  hrs::unsized_free_block_chain<vk::DeviceSize>&& _free_blocks);
    public:
        DataIndexStorage(Device* _parent_device = {},
                         std::uint32_t _rounding_indices_count = {},
                         const std::function<NewPoolSizeCalculator>& _calc =
                             MemoryType::DefaultNewPoolSizeCalculator) noexcept;

        hrs::error Recreate(std::uint32_t count, std::uint32_t init_indices_count);

        ~DataIndexStorage();
        DataIndexStorage(DataIndexStorage&& storage) noexcept;
        DataIndexStorage& operator=(DataIndexStorage&& storage) noexcept;

        void Destroy();
        bool IsCreated() const noexcept;

        vk::DeviceSize GetActualSize() const noexcept;

        Device* GetParentDevice() noexcept;
        const Device* GetParentDevice() const noexcept;
        const std::function<NewPoolSizeCalculator>& GetNewPoolSizeCalculator() const noexcept;

        void AddPool(IndexPool* pool,
                     bool copy_data); //add and realloc(after remove)
        void RemovePool(const IndexPool* pool);
        void UpdatePool(IndexPool* pool);

        bool IsSyncNeeded(std::uint32_t index) const noexcept;
        hrs::error SyncAndWrite(std::uint32_t index);
        vk::Buffer GetBuffer(std::uint32_t index) const noexcept;
    private:
        bool is_actual(std::uint32_t index) const noexcept;
        std::uint32_t get_actual_index(std::uint32_t index) const noexcept;
        hrs::expected<BoundedBufferSize, hrs::error> allocate_buffer(vk::DeviceSize size);
    private:
        Device* parent_device;
        std::vector<BoundedBufferSize> index_buffers;
        std::uint64_t actual_indices_mask;
        std::uint32_t rounding_indices_count;
        hrs::unsized_free_block_chain<vk::DeviceSize> free_blocks;
        std::function<NewPoolSizeCalculator> calc;

        std::vector<AddPoolOp> pending_adds;
        std::vector<IndexPool*> pending_updates;
    };
};
