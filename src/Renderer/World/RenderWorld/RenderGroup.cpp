#include "RenderGroup.h"
#include "../../DataIndexStorage/DataIndexStorage.h"
#include "Mesh.h"

namespace FireLand
{
    RenderGroup::RenderGroup(MaterialGroup* _parent_material_group,
                             const Mesh* _mesh,
                             IndexPool&& _pool,
                             bool _enabled)
        : PlainStateful(_enabled),
          parent_material_group(_parent_material_group),
          pool(std::move(_pool)),
          mesh(_mesh)
    {}

    RenderGroup::~RenderGroup()
    {
        destroy();
    }

    RenderGroup::RenderGroup(RenderGroup&& rg) noexcept
        : PlainStateful(std::move(rg)),
          parent_material_group(rg.parent_material_group),
          pool(std::move(rg.pool)),
          mesh(rg.mesh)
    {}

    RenderGroup& RenderGroup::operator=(RenderGroup&& rg) noexcept
    {
        destroy();

        PlainStateful::operator=(std::move(rg));
        parent_material_group = rg.parent_material_group;
        pool = std::move(rg.pool);
        mesh = rg.mesh;
        return *this;
    }

    bool RenderGroup::IsRenderable() const noexcept
    {
        return GetState() && pool.HasData();
    }

    bool RenderGroup::Render(const Mesh* prev_mesh, vk::CommandBuffer command_buffer) const noexcept
    {
        if(!IsRenderable())
            return false;

        auto [vertex_buffer, vertex_offset] = mesh->GetVertexBuffer();
        auto [index_buffer, index_offset] = mesh->GetIndexBuffer();
        if(prev_mesh == nullptr)
        {
            if(index_buffer)
                command_buffer.bindIndexBuffer(index_buffer, index_offset, vk::IndexType::eUint32);

            if(vertex_buffer)
                command_buffer.bindVertexBuffers(0, 1, &vertex_buffer, &vertex_offset);
        }
        else
        {
            if(mesh->IsIndexBufferRebindNeeded(prev_mesh) && index_buffer)
                command_buffer.bindIndexBuffer(index_buffer, index_offset, vk::IndexType::eUint32);

            if(mesh->IsVertexBufferRebindNeeded(prev_mesh) && vertex_buffer)
                command_buffer.bindVertexBuffers(0, 1, &vertex_buffer, &vertex_offset);
        }

        mesh->Render(command_buffer);
        return true;
    }

    void RenderGroup::Sync()
    {
        pool.Sync();
    }

    void RenderGroup::AcquireIndex(std::uint32_t data_index, std::uint32_t* subscriber_ptr)
    {
        pool.NewAddOp(data_index, subscriber_ptr);
    }

    void RenderGroup::RemoveIndex(std::uint32_t index) noexcept
    {
        pool.NewRemoveOp(index);
    }

    MaterialGroup* RenderGroup::GetParentMaterialGroup() noexcept
    {
        return parent_material_group;
    }

    const MaterialGroup* RenderGroup::GetParentMaterialGroup() const noexcept
    {
        return parent_material_group;
    }

    const Mesh* RenderGroup::GetMesh() const noexcept
    {
        return mesh;
    }

    void RenderGroup::destroy() noexcept
    {
        pool.GetParentStorage()->RemovePool(&pool);
    }
};
