#pragma once

#include "MemoryType.h"

namespace FireLand
{
    struct BoundedBlock
    {
        MemoryTypeAcquireResult acquire_data;
        std::vector<MemoryType>::iterator memory_type;

        BoundedBlock(const MemoryTypeAcquireResult& _acquire_data = {},
                     std::vector<MemoryType>::iterator _memory_type = {}) noexcept;

        ~BoundedBlock() = default;
        BoundedBlock(const BoundedBlock&) = default;
        BoundedBlock(BoundedBlock&&) = default;
        BoundedBlock& operator=(const BoundedBlock&) = default;
        BoundedBlock& operator=(BoundedBlock&&) = default;

        std::byte* GetBufferMapPtr() noexcept;
        const std::byte* GetBufferMapPtr() const noexcept;
    };

    struct BoundedBuffer : public BoundedBlock, public hrs::non_copyable
    {
        VkBuffer buffer;

        BoundedBuffer(const BoundedBlock& bb, VkBuffer _buffer = VK_NULL_HANDLE) noexcept;

        BoundedBuffer(const MemoryTypeAcquireResult& _acquire_data = {},
                      std::vector<MemoryType>::iterator _memory_type = {},
                      VkBuffer _buffer = VK_NULL_HANDLE) noexcept;

        ~BoundedBuffer() = default;

        BoundedBuffer(BoundedBuffer&& buf) noexcept;
        BoundedBuffer& operator=(BoundedBuffer&& buf) noexcept;

        bool IsCreated() const noexcept;
    };

    struct BoundedImage : public BoundedBlock, public hrs::non_copyable
    {
        ResourceType image_type;
        VkImage image;

        BoundedImage(const BoundedBlock& bb,
                     ResourceType _image_type = {},
                     VkImage _image = VK_NULL_HANDLE) noexcept;

        BoundedImage(const MemoryTypeAcquireResult& _acquire_data = {},
                     std::vector<MemoryType>::iterator _memory_type = {},
                     ResourceType _image_type = {},
                     VkImage _image = VK_NULL_HANDLE) noexcept;

        ~BoundedImage() = default;

        BoundedImage(BoundedImage&& img) noexcept;
        BoundedImage& operator=(BoundedImage&& img) noexcept;

        bool IsCreated() const noexcept;
    };
};
