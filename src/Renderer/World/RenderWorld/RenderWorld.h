#pragma once

#include "../../Allocator/MemoryType.h"
#include "../../TransferChannel/Data.h"
#include "../../Vulkan/VulkanInclude.hpp"
#include "RenderGroup.h"
#include "RenderPass.h"
#include "hrs/block.hpp"
#include "hrs/error.hpp"
#include "hrs/expected.hpp"
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

namespace FireLand
{
    class Mesh;
    class Material;
    class Shader;
    class RenderPass;
    class Device;

    class RenderPassPayload : hrs::non_copyable
    {
    public:
        RenderPassPayload(vk::Device _device_handle,
                          vk::CommandPool _command_pool,
                          std::vector<vk::CommandBuffer>&& _command_buffers) noexcept
            : device_handle(_device_handle),
              command_pool(_command_pool),
              command_buffers(std::move(_command_buffers))
        {}

        ~RenderPassPayload()
        {
            if(*this)
            {
                device_handle.destroy(command_pool);
                command_pool = VK_NULL_HANDLE;
            }
        }

        RenderPassPayload(RenderPassPayload&&) = default;
        RenderPassPayload& operator=(RenderPassPayload&&) = default;

        std::pair<vk::CommandPool, std::vector<vk::CommandBuffer>> Release() noexcept
        {
            return {std::exchange(command_pool, VK_NULL_HANDLE), std::move(command_buffers)};
        }

        explicit operator bool() const noexcept
        {
            return device_handle && command_pool;
        }
    private:
        vk::Device device_handle;
        vk::CommandPool command_pool;
        std::vector<vk::CommandBuffer> command_buffers;
    };

    class RenderWorld : public hrs::non_copyable
    {
    public:
        using RenderPassesContainer = std::multimap<std::size_t, std::unique_ptr<RenderPass>>;
        using RenderPassesSearchContainer =
            std::unordered_map<const RenderPass*, RenderPassesContainer::iterator>;

        RenderWorld(Device* _parent_device,
                    std::uint32_t _frame_count,
                    std::uint32_t _queue_family_index,
                    const std::function<NewPoolSizeCalculator>& _calc) noexcept;
        ~RenderWorld();
        RenderWorld(RenderWorld&& rw) noexcept;
        RenderWorld& operator=(RenderWorld&& rw) noexcept;

        /*RenderGroup * GetRenderGroup(const Material *material, const Mesh *mesh) const noexcept;

		RenderGroup * NotifyNewRenderGroup(const Material *material,
										   const Mesh *mesh,
										   const RenderGroupOptions &options);
		*/
        //void NotifyRemoveEmptyRenderGroup(RenderGroup *render_group);
        //void NotifyRemoveRenderGroup(RenderGroup *render_group);

        /*(MaterialGroup *_parent_material_group,
		 const Mesh *_mesh,
		 IndexPool &&_pool,
		 bool _enabled);*/

        vk::Result RebindMaterial(const Shader* shader, const Material* mtl);
        vk::Result RebindMaterial(const MaterialGroup* mtl_group);

        void NotifyNewShaderObjectData(const Shader* shader,
                                       Data data,
                                       std::uint32_t* index_subscriber_ptr);

        void NotifyUpdateShaderObjectData(const Shader* shader,
                                          Data data,
                                          std::uint32_t index,
                                          const hrs::block<vk::DeviceSize>& data_block,
                                          vk::DeviceSize in_data_buffer_offset);

        void NotifyRemoveShaderObjectData(const Shader* shader, std::uint32_t index);

        void NotifyNewRenderGroupInstance(const RenderGroup* render_group,
                                          std::uint32_t data_index,
                                          std::uint32_t* subscriber_ptr);

        void NotifyRemoveRenderGroupInstance(const RenderGroup* render_group, std::uint32_t index);

        RenderGroup* AddRenderGroup(const Shader* shader,
                                    const Material* mtl,
                                    const Mesh* mesh,
                                    std::uint32_t init_size_power,
                                    std::uint32_t rounding_size,
                                    bool _enabled);

        hrs::error Flush(std::uint32_t frame_index);

        hrs::expected<std::span<const vk::CommandBuffer>, vk::Result>
        Render(std::uint32_t frame_index, vk::DescriptorSet globals_set) const noexcept;

        hrs::expected<RenderPassPayload, vk::Result> AcquireRenderPassPayload();
        bool AddRenderPass(RenderPass* renderpass, std::size_t priority);

        bool HasRenderPass(const RenderPass* renderpass) const noexcept;
        void RemoveRenderPass(const RenderPass* rpass) noexcept;

        const std::function<NewPoolSizeCalculator>& GetNewPoolSizeCalculator() const noexcept;
        Device* GetParentDevice() noexcept;
        const Device* GetParentDevice() const noexcept;
        std::uint32_t GetFrameCount() const noexcept;
    private:
        void destroy() noexcept;
    private:
        Device* parent_device;
        std::uint32_t frame_count;
        std::uint32_t queue_family_index;
        std::function<NewPoolSizeCalculator> calc;
        mutable std::vector<std::vector<vk::CommandBuffer>> render_results;
        RenderPassesContainer renderpasses;
        RenderPassesSearchContainer renderpasses_search;
        //renderpass -> shader -> material -> mesh -> render_group
    };
};
