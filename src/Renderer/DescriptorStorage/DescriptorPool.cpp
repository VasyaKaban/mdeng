#include "DescriptorPool.h"
#include "../Context/DeviceLoader.h"
#include "DescriptorStorage.h"
#include "hrs/debug.hpp"

namespace FireLand
{
    DescriptorPool::DescriptorPool(DescriptorStorage* _storage,
                                   VkDescriptorPool _pool,
                                   std::uint32_t _max_set_count) noexcept
        : storage(_storage),
          pool(_pool),
          max_set_count(_max_set_count),
          issued_sets_count(0)
    {}

    hrs::expected<DescriptorPool, VkResult>
    DescriptorPool::Create(DescriptorStorage& _parent_storage,
                           const VkDescriptorPoolCreateInfo& info) noexcept
    {
        hrs::assert_true_debug(_parent_storage.IsCreated(),
                               "Descriptor storage isn't created yet!");

        VkDescriptorPool _pool;
        VkResult res = _parent_storage.GetDeviceLoader()->vkCreateDescriptorPool(
            _parent_storage.GetDevice(),
            &info,
            _parent_storage.GetAllocationCallbacks(),
            &_pool);
        if(res != VK_SUCCESS)
            return res;

        return DescriptorPool(&_parent_storage, _pool, info.maxSets);
    }

    DescriptorPool::~DescriptorPool()
    {
        Destroy();
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& d_pool) noexcept
        : storage(d_pool.storage),
          pool(std::exchange(d_pool.pool, VK_NULL_HANDLE)),
          free_sets(std::move(d_pool.free_sets)),
          max_set_count(d_pool.max_set_count),
          issued_sets_count(d_pool.issued_sets_count)
    {}

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& d_pool) noexcept
    {
        Destroy();

        storage = d_pool.storage;
        pool = std::exchange(d_pool.pool, VK_NULL_HANDLE);
        free_sets = std::move(d_pool.free_sets);
        max_set_count = d_pool.max_set_count;
        issued_sets_count = d_pool.issued_sets_count;

        return *this;
    }

    void DescriptorPool::Destroy() noexcept
    {
        if(!IsCreated())
            return;

        ResetPool();
        storage->GetDeviceLoader()->vkDestroyDescriptorPool(storage->GetDevice(),
                                                            pool,
                                                            storage->GetAllocationCallbacks());
        pool = VK_NULL_HANDLE;
    }

    bool DescriptorPool::IsCreated() const noexcept
    {
        return pool != VK_NULL_HANDLE;
    }

    hrs::expected<std::vector<VkDescriptorSet>, VkResult> DescriptorPool::AllocateSets()
    {
        hrs::assert_true_debug(IsCreated(), "Descriptor pool isn't created yet!");

        std::uint32_t count = storage->GetSetLayoutCount();
        if(free_sets.size() >= count)
        {
            auto start_it = free_sets.begin() + (free_sets.size() - count);
            std::vector<VkDescriptorSet> sets(start_it, free_sets.end());
            free_sets.erase(start_it, free_sets.end());
            issued_sets_count += count;
            return sets;
        }
        else
        {
            const VkDescriptorSetAllocateInfo info = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .pNext = nullptr,
                .descriptorPool = pool,
                .descriptorSetCount = count,
                .pSetLayouts = storage->GetDescriptorSetLayouts().data()};

            std::vector<VkDescriptorSet> sets(count, VK_NULL_HANDLE);
            VkResult res =
                storage->GetDeviceLoader()->vkAllocateDescriptorSets(storage->GetDevice(),
                                                                     &info,
                                                                     sets.data());
            if(res != VK_SUCCESS)
                return res;

            issued_sets_count += count;
            return res;
        }
    }

    void DescriptorPool::ResetPool() noexcept
    {
        hrs::assert_true_debug(IsCreated(), "Descriptor pool isn't created yet!");

        [[maybe_unused]] VkResult _ =
            storage->GetDeviceLoader()->vkResetDescriptorPool(storage->GetDevice(), pool, {});

        issued_sets_count = 0;
        free_sets.clear();
    }

    void DescriptorPool::RetireSets(std::span<const VkDescriptorSet> sets)
    {
        hrs::assert_true_debug(IsCreated(), "Descripor pool isn't created yet!");
        hrs::assert_true_debug(max_set_count >= sets.size() && issued_sets_count >= sets.size(),
                               "Issued sets count: {}"
                               " and max set count: {}"
                               " both must be greater than or equal to "
                               "requested free sets count: {}",
                               issued_sets_count,
                               max_set_count,
                               sets.size());

        if(sets.empty())
            return;

        free_sets.insert(free_sets.end(), sets.begin(), sets.end());
        issued_sets_count -= sets.size();
    }

    void DescriptorPool::FreeSets(std::span<const VkDescriptorSet> sets) noexcept
    {
        hrs::assert_true_debug(IsCreated(), "Descriptor pool isn't created yet!");
        hrs::assert_true_debug(max_set_count >= sets.size() && issued_sets_count >= sets.size(),
                               "Issued sets count: {}"
                               " and max set count: {}"
                               " both must be greater than or equal to "
                               "requested free sets count: {}",
                               issued_sets_count,
                               max_set_count,
                               sets.size());

        if(sets.empty())
            return;

        [[maybe_unused]] VkResult _ =
            storage->GetDeviceLoader()->vkFreeDescriptorSets(storage->GetDevice(),
                                                             pool,
                                                             sets.size(),
                                                             sets.data());

        issued_sets_count -= sets.size();
    }

    DescriptorStorage* DescriptorPool::GetStorage() noexcept
    {
        return storage;
    }

    const DescriptorStorage* DescriptorPool::GetStorage() const noexcept
    {
        return storage;
    }

    VkDescriptorPool DescriptorPool::GetHandle() const noexcept
    {
        return pool;
    }

    std::uint32_t DescriptorPool::GetMaxSetCount() const noexcept
    {
        return max_set_count;
    }

    std::uint32_t DescriptorPool::GetIssuedSetCount() const noexcept
    {
        return issued_sets_count;
    }

    std::uint32_t DescriptorPool::GetFreeSetCount() const noexcept
    {
        return free_sets.size();
    }
};
