#pragma once

#include "../../DataIndexStorage/IndexPool.h"
#include "../../Vulkan/VulkanInclude.hpp"
#include "PlainStateful.h"
#include "hrs/error.hpp"
#include <cstdint>

namespace FireLand
{
    class Mesh;
    class MaterialGroup;
    class DataIndexStorage;

    class RenderGroup : public hrs::non_copyable, public PlainStateful
    {
    public:
        RenderGroup(MaterialGroup* _parent_material_group,
                    const Mesh* _mesh,
                    IndexPool&& _pool,
                    bool _enabled);

        ~RenderGroup();
        RenderGroup(RenderGroup&& rg) noexcept;
        RenderGroup& operator=(RenderGroup&& rg) noexcept;

        bool IsRenderable() const noexcept;
        bool Render(const Mesh* prev_mesh, vk::CommandBuffer command_buffer) const noexcept;
        void Sync();

        void AcquireIndex(std::uint32_t data_index, std::uint32_t* subscriber_ptr);
        void RemoveIndex(std::uint32_t index) noexcept;

        MaterialGroup* GetParentMaterialGroup() noexcept;
        const MaterialGroup* GetParentMaterialGroup() const noexcept;
        const Mesh* GetMesh() const noexcept;
    private:
        void destroy() noexcept;
    private:
        MaterialGroup* parent_material_group;
        IndexPool pool;
        const Mesh* mesh;
    };
};
