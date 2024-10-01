#include "Allocator.h"
#include "../Context/DeviceLoader.h"
#include "../Context/InstanceLoader.h"
#include "../Vulkan/codegen/loader_check_begin.h"
#include "Bounded.h"
#include <ranges>

namespace FireLand
{
    Allocator::Allocator(VkDevice _device,
                         const DeviceLoader* _dl,
                         VkDeviceSize _buffer_image_granularity,
                         std::vector<MemoryType>&& _memory_types,
                         std::function<NewPoolSizeCalculator>&& _pool_size_calc,
                         const VkAllocationCallbacks* _allocation_callbacks) noexcept
        : device(_device),
          dl(_dl),
          buffer_image_granularity(_buffer_image_granularity),
          memory_types(std::move(_memory_types)),
          pool_size_calc(std::move(_pool_size_calc)),
          allocation_callbacks(_allocation_callbacks)
    {}

    Allocator::Allocator() noexcept
        : device(VK_NULL_HANDLE)
    {}

    Allocator::~Allocator()
    {
        Destroy();
    }

    Allocator::Allocator(Allocator&& allocator) noexcept
        : device(std::exchange(allocator.device, VK_NULL_HANDLE)),
          dl(allocator.dl),
          buffer_image_granularity(allocator.buffer_image_granularity),
          memory_types(std::move(allocator.memory_types)),
          pool_size_calc(std::move(allocator.pool_size_calc)),
          allocation_callbacks(allocator.allocation_callbacks)
    {}

    Allocator& Allocator::operator=(Allocator&& allocator) noexcept
    {
        Destroy();

        device = std::exchange(allocator.device, VK_NULL_HANDLE);
        dl = allocator.dl;
        buffer_image_granularity = allocator.buffer_image_granularity;
        memory_types = std::move(allocator.memory_types);
        pool_size_calc = std::move(allocator.pool_size_calc);
        allocation_callbacks = allocator.allocation_callbacks;

        return *this;
    }

    hrs::expected<Allocator, InitResult>
    Allocator::Create(VkDevice _device,
                      VkPhysicalDevice physical_device,
                      const DeviceLoader& _dl,
                      const InstanceLoader& il,
                      std::function<NewPoolSizeCalculator>&& _pool_size_calc,
                      const VkAllocationCallbacks* _allocation_callbacks)
    {
        hrs::assert_true_debug(_device != VK_NULL_HANDLE, "Device isn't created yet!");
        hrs::assert_true_debug(physical_device != VK_NULL_HANDLE,
                               "Physical device isn't created yet!");

        FIRE_LAND_LOADER_CHECK_USE(il)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkGetPhysicalDeviceProperties)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkGetPhysicalDeviceMemoryProperties)
        FIRE_LAND_LOADER_CHECK_UNUSE()
        FIRE_LAND_LOADER_CHECK_USE(_dl)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkAllocateMemory)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkFreeMemory)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkUnmapMemory)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkMapMemory)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkCreateBuffer)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkBindBufferMemory)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkGetBufferMemoryRequirements)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkDestroyBuffer)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkCreateImage)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkBindImageMemory)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkGetImageMemoryRequirements)
            FIRE_LAND_LOADER_CHECK_FUNCTION(vkDestroyImage)
        FIRE_LAND_LOADER_CHECK_UNUSE()

        VkPhysicalDeviceProperties props;
        il.vkGetPhysicalDeviceProperties(physical_device, &props);
        VkDeviceSize buffer_image_granularity = props.limits.bufferImageGranularity;

        VkPhysicalDeviceMemoryProperties mem_props;
        il.vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

        std::vector<MemoryType> _memory_types;
        _memory_types.reserve(mem_props.memoryTypeCount);
        for(std::size_t i = 0; i < mem_props.memoryTypeCount; i++)
        {
            const auto& mem_type = mem_props.memoryTypes[i];
            _memory_types.emplace_back(mem_props.memoryHeaps[mem_type.heapIndex],
                                       mem_type.propertyFlags,
                                       i,
                                       buffer_image_granularity);
        }

        return Allocator(_device,
                         &_dl,
                         buffer_image_granularity,
                         std::move(_memory_types),
                         std::move(_pool_size_calc),
                         _allocation_callbacks);
    }

    void Allocator::Destroy() noexcept
    {
        for(auto& mem_type: memory_types)
            mem_type.Destroy(device, *dl, allocation_callbacks);

        memory_types.clear();
        allocation_callbacks = nullptr;
        device = VK_NULL_HANDLE;
    }

    VkDevice Allocator::GetDevice() const noexcept
    {
        return device;
    }

    const DeviceLoader* Allocator::GetDeviceLoader() const noexcept
    {
        return dl;
    }

    const std::vector<MemoryType>& Allocator::GetMemoryTypes() const noexcept
    {
        return memory_types;
    }

    const std::function<NewPoolSizeCalculator>&
    Allocator::GetPoolSizeCalculatorFunction() const noexcept
    {
        return pool_size_calc;
    }

    void
    Allocator::SetPoolSizeCalculatorFunction(std::function<NewPoolSizeCalculator>&& _pool_new_calc)
    {
        pool_size_calc = std::move(_pool_new_calc);
    }

    VkDeviceSize Allocator::GetBufferImageGranularity() const noexcept
    {
        return buffer_image_granularity;
    }

    bool Allocator::IsGranularityFree() const noexcept
    {
        return buffer_image_granularity == 1;
    }

    bool Allocator::IsCreated() const noexcept
    {
        return device != VK_NULL_HANDLE;
    }

    hrs::expected<std::pair<BoundedBlock, std::size_t>, hrs::error>
    Allocator::Allocate(const VkMemoryRequirements& req,
                        ResourceType res_type,
                        std::span<const MultipleAllocateDesiredOptions> desired,
                        const std::function<NewPoolSizeCalculator>& calc)
    {
        hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");

        hrs::error err = AllocatorResult::NoSatisfiedMemoryTypes;
        for(std::size_t i = 0; i < desired.size(); i++)
        {
            const auto& mado = desired[i];
            hrs::assert_true_debug((mado.flags & AllocationFlags::AllocateSeparatePool &
                                    AllocationFlags::CreateOnExistedPools) ?
                                       false :
                                       true,
                                   "Conflicting flags: AllocateSeparatePool "
                                   "and CreateOnExistedPools!");

            if(mado.flags & AllocationFlags::CreateOnExistedPools ||
               !(mado.flags & AllocationFlags::AllocateSeparatePool))
            {
                //only call TryAcquire
                for(auto mem_type_it = memory_types.begin(); mem_type_it != memory_types.end();
                    mem_type_it++)
                {
                    if(is_memory_type_satisfy(*mem_type_it, mado.op, mado.memory_property, req))
                    {
                        auto alloc_exp = mem_type_it->TryAcquire(
                            res_type,
                            req,
                            mado.flags,
                            device,
                            *dl,
                            allocation_callbacks,
                            (calc ? calc : MemoryType::DefaultNewPoolSizeCalculator));

                        if(alloc_exp)
                            return std::pair{BoundedBlock(*alloc_exp, mem_type_it), i};
                        else
                            err = alloc_exp.error();
                    }
                }
            }

            if(mado.flags & AllocationFlags::AllocateSeparatePool ||
               !(mado.flags & AllocationFlags::CreateOnExistedPools))
            {
                //only call TryAllocate
                for(auto mem_type_it = memory_types.begin(); mem_type_it != memory_types.end();
                    mem_type_it++)
                {
                    if(is_memory_type_satisfy(*mem_type_it, mado.op, mado.memory_property, req))
                    {
                        auto alloc_exp = mem_type_it->TryAllocate(
                            res_type,
                            req,
                            mado.flags,
                            device,
                            *dl,
                            allocation_callbacks,
                            (calc ? calc : MemoryType::DefaultNewPoolSizeCalculator));

                        if(alloc_exp)
                            return std::pair{BoundedBlock(*alloc_exp, mem_type_it), i};
                        else
                            err = alloc_exp.error();
                    }
                }
            }
        }

        return err;
    }

    void
    Allocator::Free(const BoundedBlock& bb, MemoryPoolOnEmptyPolicy policy, ResourceType res_type)
    {
        hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");
        hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(memory_types, bb.memory_type),
                               "Memory type isn't a part of this allocator!");

        bb.memory_type
            ->Release(res_type, bb.acquire_data, policy, device, *dl, allocation_callbacks);
    }

    hrs::expected<std::pair<BoundedBuffer, std::size_t>, hrs::error>
    Allocator::Allocate(const VkBufferCreateInfo& info,
                        std::span<const MultipleAllocateDesiredOptions> desired,
                        const std::function<NewPoolSizeCalculator>& calc)
    {
        hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");

        VkBuffer buffer;
        VkResult res = dl->vkCreateBuffer(device, &info, allocation_callbacks, &buffer);
        if(res != VK_SUCCESS)
            return res;

        VkMemoryRequirements req;
        dl->vkGetBufferMemoryRequirements(device, buffer, &req);
        auto alloc_exp = Allocate(req, ResourceType::Linear, desired, calc);
        if(!alloc_exp)
        {
            dl->vkDestroyBuffer(device, buffer, allocation_callbacks);
            return alloc_exp.error();
        }

        const auto& acq_data = alloc_exp->first.acquire_data;
        res = dl->vkBindBufferMemory(device,
                                     buffer,
                                     acq_data.pool->GetMemory().GetDeviceMemory(),
                                     acq_data.block.offset);

        if(res != VK_SUCCESS)
        {
            //use policy = free because if pool is newly created then it must be deleted due to we need to return to the state as it was before allocation
            Free(alloc_exp->first, MemoryPoolOnEmptyPolicy::Free, ResourceType::Linear);
            dl->vkDestroyBuffer(device, buffer, allocation_callbacks);
            return res;
        }

        return std::pair{BoundedBuffer(alloc_exp->first, buffer), alloc_exp->second};
    }

    void Allocator::Free(const BoundedBuffer& bounded_buffer, MemoryPoolOnEmptyPolicy policy)
    {
        if(!bounded_buffer.IsCreated())
            return;

        Free(bounded_buffer, policy, ResourceType::Linear);
        dl->vkDestroyBuffer(device, bounded_buffer.buffer, allocation_callbacks);
    }

    hrs::expected<std::pair<BoundedImage, std::size_t>, hrs::error>
    Allocator::Allocate(const VkImageCreateInfo& info,
                        std::span<const MultipleAllocateDesiredOptions> desired,
                        const std::function<NewPoolSizeCalculator>& calc)
    {
        hrs::assert_true_debug(IsCreated(), "Allocator isn't created yet!");

        VkImage image;
        VkResult res = dl->vkCreateImage(device, &info, allocation_callbacks, &image);
        if(res != VK_SUCCESS)
            return res;

        VkMemoryRequirements req;
        dl->vkGetImageMemoryRequirements(device, image, &req);

        hrs::assert_true_debug(info.tiling != VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT,
                               "Allocator doesn't support "
                               "VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT! "
                               "Use LINEAR or OPTIMAL tiling instead!");

        ResourceType res_type = (info.tiling == VK_IMAGE_TILING_LINEAR ? ResourceType::Linear :
                                                                         ResourceType::NonLinear);

        auto alloc_exp = Allocate(req, res_type, desired, calc);
        if(!alloc_exp)
        {
            dl->vkDestroyImage(device, image, allocation_callbacks);
            return alloc_exp.error();
        }

        const auto& acq_data = alloc_exp->first.acquire_data;
        res = dl->vkBindImageMemory(device,
                                    image,
                                    acq_data.pool->GetMemory().GetDeviceMemory(),
                                    acq_data.block.offset);

        if(res != VK_SUCCESS)
        {
            Free(alloc_exp->first, MemoryPoolOnEmptyPolicy::Free, res_type);
            dl->vkDestroyImage(device, image, allocation_callbacks);
            return res;
        }

        return std::pair{BoundedImage(alloc_exp->first, res_type, image), alloc_exp->second};
    }

    void Allocator::Free(const BoundedImage& bounded_image, MemoryPoolOnEmptyPolicy policy)
    {
        if(!bounded_image.IsCreated())
            return;

        Free(bounded_image, policy, bounded_image.image_type);
        dl->vkDestroyImage(device, bounded_image.image, allocation_callbacks);
    }

    bool Allocator::is_memory_type_satisfy(const MemoryType& mem_type,
                                           MemoryTypeSatisfyOp op,
                                           VkMemoryPropertyFlags desired_props,
                                           const VkMemoryRequirements& req) const noexcept
    {
        return mem_type.IsSatisfyIndex(req.memoryTypeBits) && mem_type.IsSatisfy(op, desired_props);
    }
};

#include "../Vulkan/codegen/loader_check_end.h"
