#include "Memory.h"
#include "../Context/DeviceLoader.h"
#include "hrs/debug.hpp"
#include <utility>

namespace FireLand
{
    Memory::Memory(VkDeviceMemory _memory, VkDeviceSize _size, std::byte* _map_ptr) noexcept
        : memory(_memory),
          size(_size),
          map_ptr(_map_ptr)
    {}

    Memory::Memory(Memory&& mem) noexcept
        : memory(std::exchange(mem.memory, VK_NULL_HANDLE)),
          size(std::exchange(mem.size, 0)),
          map_ptr(std::exchange(mem.map_ptr, nullptr))
    {}

    void
    Memory::Free(VkDevice device, const DeviceLoader& dl, const VkAllocationCallbacks* alc) noexcept
    {
        hrs::assert_true_debug(device != VK_NULL_HANDLE, "Device isn't created yet!");
        if(IsMapped())
        {
            dl.vkUnmapMemory(device, memory);
            map_ptr = nullptr;
        }

        if(IsAllocated())
        {
            dl.vkFreeMemory(device, memory, alc);
            memory = VK_NULL_HANDLE;
            size = 0;
        }
    }

    bool Memory::IsAllocated() const noexcept
    {
        return memory != VK_NULL_HANDLE;
    }

    bool Memory::IsMapped() const noexcept
    {
        return map_ptr;
    }

    VkDeviceMemory Memory::GetDeviceMemory() const noexcept
    {
        return memory;
    }

    VkDeviceSize Memory::GetSize() const noexcept
    {
        return size;
    }

    VkResult Memory::MapMemory(VkDevice device, const DeviceLoader& dl) noexcept
    {
        if(IsMapped())
            return VK_SUCCESS;

        hrs::assert_true_debug(device, "Device isn't created yet!");
        hrs::assert_true_debug(IsAllocated(), "Memory isn't allocated yet!");

        void* ptr;
        VkResult res = dl.vkMapMemory(device, memory, 0, size, {}, &ptr);
        if(res != VK_SUCCESS)
            return res;

        map_ptr = reinterpret_cast<std::byte*>(ptr);
        return VK_SUCCESS;
    }
};
