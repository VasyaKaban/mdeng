#include "DescriptorStorage.h"
#include "../Context/DeviceLoader.h"
#include "../Vulkan/codegen/loader_check_begin.h"
#include "DescriptorPool.h"
#include "hrs/debug.hpp"

namespace FireLand
{
    DescriptorStorage::DescriptorStorage(
        VkDevice _device,
        const DeviceLoader* _dl,
        const VkAllocationCallbacks* _allocation_callbacks,
        std::vector<VkDescriptorSetLayout>&& _descriptor_set_layouts,
        DescriptorPoolInfo&& _pool_info) noexcept
        : device(_device),
          dl(_dl),
          allocation_callbacks(_allocation_callbacks),
          descriptor_set_layouts(std::move(_descriptor_set_layouts)),
          pool_info(std::move(_pool_info))
    {}

    hrs::expected<DescriptorStorage, InitResult>
    DescriptorStorage::Create(VkDevice _device,
                              const DeviceLoader& _dl,
                              const VkDescriptorSetLayoutCreateInfo& set_layout_info,
                              std::uint32_t set_layout_count,
                              DescriptorPoolInfo&& _pool_info,
                              const VkAllocationCallbacks* _allocation_callbacks)
    {
        hrs::assert_true_debug(_device != VK_NULL_HANDLE, "Device isn't created yet!");
        hrs::assert_true_debug(set_layout_count != 0, "Count of layouts must greater than zero!");

        FIRE_LAND_LOADER_CHECK_USE(_dl)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkCreateDescriptorSetLayout)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkDestroyDescriptorSetLayout)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkCreateDescriptorPool)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkResetDescriptorPool)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkDestroyDescriptorPool)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkAllocateDescriptorSets)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkFreeDescriptorSets)
        FIRE_LAND_LOADER_CHECK_UNUSE()

        VkDescriptorSetLayout set_layout;
        VkResult res = _dl.vkCreateDescriptorSetLayout(_device,
                                                       &set_layout_info,
                                                       _allocation_callbacks,
                                                       &set_layout);

        if(res != VK_SUCCESS)
            return res;

        return DescriptorStorage(_device,
                                 &_dl,
                                 _allocation_callbacks,
                                 std::vector<VkDescriptorSetLayout>(set_layout_count, set_layout),
                                 std::move(_pool_info));
    }

    DescriptorStorage::~DescriptorStorage()
    {
        Destroy();
    }

    DescriptorStorage::DescriptorStorage(DescriptorStorage&& storage) noexcept
        : device(std::exchange(storage.device, VK_NULL_HANDLE)),
          dl(storage.dl),
          allocation_callbacks(storage.allocation_callbacks),
          descriptor_set_layouts(std::move(storage.descriptor_set_layouts)),
          pool_info(std::move(storage.pool_info)),
          pools(std::move(storage.pools))
    {}

    DescriptorStorage& DescriptorStorage::operator=(DescriptorStorage&& storage) noexcept
    {
        Destroy();

        device = std::exchange(storage.device, VK_NULL_HANDLE);
        dl = storage.dl;
        allocation_callbacks = storage.allocation_callbacks;
        descriptor_set_layouts = std::move(storage.descriptor_set_layouts);
        pool_info = std::move(storage.pool_info);
        pools = std::move(storage.pools);

        return *this;
    }

    void DescriptorStorage::Destroy()
    {
        if(!IsCreated())
            return;

        pools.clear();
        pool_info = {};
        dl->vkDestroyDescriptorSetLayout(device, descriptor_set_layouts[0], allocation_callbacks);
        descriptor_set_layouts.clear();
        device = VK_NULL_HANDLE;
    }

    bool DescriptorStorage::IsCreated() const noexcept
    {
        return device != VK_NULL_HANDLE;
    }

    hrs::expected<DescriptorSetGroup, VkResult> DescriptorStorage::AllocateSetGroup()
    {
        hrs::assert_true_debug(IsCreated(), "Descriptor storage isn't created yet!");

        for(auto& pool: pools)
        {
            auto alloc_exp = pool.AllocateSets();
            if(alloc_exp)
                return DescriptorSetGroup{std::move(*alloc_exp), &pool};
        }

        const VkDescriptorPoolCreateInfo pool_create_info = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = pool_info.flags,
            .maxSets = pool_info.max_sets,
            .poolSizeCount = static_cast<std::uint32_t>(pool_info.pool_sizes.size()),
            .pPoolSizes = pool_info.pool_sizes.data()};

        auto pool_exp = DescriptorPool::Create(*this, pool_create_info);
        if(!pool_exp)
            return pool_exp.error();

        auto alloc_exp = pool_exp->AllocateSets();
        if(!alloc_exp)
            return alloc_exp.error();

        pools.push_back(std::move(*pool_exp));
        return DescriptorSetGroup{std::move(*alloc_exp), &pools.back()};
    }

    void DescriptorStorage::RetireSetGroup(const DescriptorSetGroup& group)
    {
        hrs::assert_true_debug(IsCreated(), "Descriptor storage isn't created yet!");
        hrs::assert_true_debug(group.parent_pool != nullptr, "Descriptor pool is null!");

        group.parent_pool->RetireSets(group.sets);
    }

    void DescriptorStorage::FreeSetGroup(const DescriptorSetGroup& group)
    {
        hrs::assert_true_debug(IsCreated(), "Descriptor storage isn't created yet!");
        hrs::assert_true_debug(group.parent_pool != nullptr, "Descriptor pool is null!");

        group.parent_pool->FreeSets(group.sets);
    }

    void DescriptorStorage::DestroyFoolPools() noexcept
    {
        std::erase_if(pools,
                      [](const DescriptorPool& pool)
                      {
                          return pool.GetIssuedSetCount() == 0;
                      });
    }

    VkDevice DescriptorStorage::GetDevice() const noexcept
    {
        return device;
    }

    const DeviceLoader* DescriptorStorage::GetDeviceLoader() const noexcept
    {
        return dl;
    }

    VkDescriptorSetLayout DescriptorStorage::GetDescriptorSetLayout() const noexcept
    {
        if(descriptor_set_layouts.empty())
            return VK_NULL_HANDLE;

        return descriptor_set_layouts[0];
    }

    const std::vector<VkDescriptorSetLayout>&
    DescriptorStorage::GetDescriptorSetLayouts() const noexcept
    {
        return descriptor_set_layouts;
    }

    const DescriptorPoolInfo& DescriptorStorage::GetPoolInfo() const noexcept
    {
        return pool_info;
    }

    const VkAllocationCallbacks* DescriptorStorage::GetAllocationCallbacks() const noexcept
    {
        return allocation_callbacks;
    }

    std::uint32_t DescriptorStorage::GetSetLayoutCount() const noexcept
    {
        return descriptor_set_layouts.size();
    }
};
