#pragma once

#include "../Vulkan/InitResult.h"
#include "../Vulkan/QueueFamilyIndex.h"
#include "BoundedBufferSizeFillness.h"
#include "TransferBufferOp.h"
#include "TransferImageOp.h"
#include "hrs/error.hpp"
#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include <span>

namespace FireLand
{
    class Allocator;

    enum class TransferChannelWriteState
    {
        WriteStarted, //Begin -> fence: Sig
        WriteEnded, //End -> fence: Unsig
        Flushed //Flush -> fence: submited
    };

    struct EmbedResult
    {
        VkCommandBuffer command_buffer;
        VkBuffer buffer;
        VkDeviceSize offset;
    };

    class TransferChannel : public hrs::non_copyable
    {
    private:
        TransferChannel(VkDevice _device,
                        const DeviceLoader* _dl,
                        Allocator* _allocator,
                        VkFence _wait_fence,
                        const QueueFamilyIndex& _transfer_queue,
                        VkDeviceSize _buffer_rounding_size,
                        VkCommandBuffer _command_buffer,
                        const VkAllocationCallbacks* _allocation_callbacks) noexcept;
    public:
        TransferChannel() noexcept;
        ~TransferChannel();
        TransferChannel(TransferChannel&& tc) noexcept;
        TransferChannel& operator=(TransferChannel&& tc) noexcept;

        static hrs::expected<TransferChannel, InitResult>
        Create(VkDevice _device,
               const DeviceLoader& _dl,
               Allocator& _allocator,
               const QueueFamilyIndex& _transfer_queue,
               VkCommandBuffer _command_buffer,
               VkDeviceSize _buffer_rounding_size,
               const VkAllocationCallbacks* _allocation_callbacks);

        void Destroy() noexcept;
        bool IsCreated() const noexcept;

        VkResult Flush(std::span<const VkSubmitInfo> submits) noexcept;

        VkResult GetWaitFenceStatus() const noexcept;
        VkResult
        WaitFence(std::uint64_t timeout = std::numeric_limits<std::uint64_t>::max()) noexcept;

        VkDevice GetDevice() const noexcept;
        const DeviceLoader* GetDeviceLoader() const noexcept;
        Allocator* GetAllocator() noexcept;
        const Allocator* GetAllocator() const noexcept;
        const QueueFamilyIndex& GetTransferQueue() const noexcept;
        VkDeviceSize GetBufferRoundingSize() const noexcept;
        VkCommandBuffer GetCommandBuffer() const noexcept;
        VkFence GetWaitFence() const noexcept;
        const std::vector<BoundedBufferSizeFillness>& GetBuffers() const noexcept;

        VkResult Begin(const VkCommandBufferBeginInfo& info) noexcept;
        VkResult End() noexcept;
        TransferChannelWriteState GetWriteState() const noexcept;

        hrs::error CopyBuffer(VkBuffer dst_buffer,
                              std::span<const std::byte*> datas,
                              std::span<const TransferBufferOpRegion> regions);

        //do not care about minImageTransferGranularity because we want to transfer the whole mip level!
        hrs::error CopyImageSubresource(VkImage dst_image,
                                        VkImageLayout image_layout,
                                        VkDeviceSize block_size,
                                        std::span<const std::byte*> datas,
                                        std::span<const TransferImageOpRegion> regions);

        hrs::expected<EmbedResult, hrs::error> Embed(const hrs::mem_req<VkDeviceSize>& req);

        void EmbedBarrier(VkPipelineStageFlags src_stages,
                          VkPipelineStageFlags dst_stages,
                          VkDependencyFlags dependency,
                          std::span<const VkMemoryBarrier> memory_barriers,
                          std::span<const VkBufferMemoryBarrier> buffer_memory_barriers,
                          std::span<const VkImageMemoryBarrier> image_memory_barriers) noexcept;

        hrs::error FlattenBuffers();
        hrs::error InsertBuffer(VkDeviceSize size);
    private:
        hrs::error allocate_insert_buffer(VkDeviceSize size);

        void copy_buffer(VkBuffer dst_buffer,
                         BoundedBufferSizeFillness& src_buffer,
                         std::span<const std::byte*> datas,
                         std::span<const TransferBufferOpRegion> regions,
                         VkDeviceSize offset) noexcept;

        void copy_image(VkImage dst_image,
                        VkImageLayout image_layout,
                        VkDeviceSize block_size,
                        BoundedBufferSizeFillness& src_buffer,
                        std::span<const std::byte*> datas,
                        std::span<const TransferImageOpRegion> regions,
                        VkDeviceSize offset) noexcept;

        VkDeviceSize image_data_size(const VkExtent3D& extent,
                                     VkDeviceSize block_size) const noexcept;
    private:
        VkDevice device;
        const DeviceLoader* dl;
        Allocator* allocator;
        std::vector<BoundedBufferSizeFillness> buffers;
        VkFence wait_fence;
        QueueFamilyIndex transfer_queue;
        TransferChannelWriteState write_state;
        VkDeviceSize buffer_rounding_size;
        VkCommandBuffer command_buffer;
        const VkAllocationCallbacks* allocation_callbacks;
    };
};
