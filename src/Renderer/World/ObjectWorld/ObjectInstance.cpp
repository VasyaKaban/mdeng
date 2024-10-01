#include "ObjectInstance.h"
#include "../RenderWorld/RenderWorld.h"
#include "../RenderWorld/Shader.h"
#include "Object.h"

namespace FireLand
{
    ObjectInstance::ObjectInstance(Object* _parent_object)
        : parent_object(_parent_object)
    {
        std::size_t mesh_count = parent_object->GetMeshCount();
        mesh_bindings.reserve(mesh_count);
        for(std::size_t i = 0; i < mesh_count; i++)
            mesh_bindings.emplace_back(this, parent_object->GetMesh(i));
    }

    ObjectInstance::~ObjectInstance()
    {
        destroy();
    }

    ObjectInstance::ObjectInstance(ObjectInstance&& oi) noexcept
        : parent_object(oi.parent_object),
          shader_data_bindings(std::move(oi.shader_data_bindings)),
          mesh_bindings(std::move(oi.mesh_bindings))
    {}

    ObjectInstance& ObjectInstance::operator=(ObjectInstance&& oi) noexcept
    {
        destroy();

        parent_object = oi.parent_object;
        shader_data_bindings = std::move(oi.shader_data_bindings);
        mesh_bindings = std::move(oi.mesh_bindings);

        return *this;
    }

    Object* ObjectInstance::GetParentObejct() noexcept
    {
        return parent_object;
    }

    const Object* ObjectInstance::GetParentObject() const noexcept
    {
        return parent_object;
    }

    bool ObjectInstance::HasShaderDataBinding(const Shader* shader) const noexcept
    {
        return shader_data_bindings.find(shader) != shader_data_bindings.end();
    }

    bool ObjectInstance::AddShaderDataBinding(const Shader* shader, RenderWorld* render_world)
    {
        if(shader->GetParentRenderPass()->GetParentWorld() != render_world)
            return false;

        auto it = shader_data_bindings.find(shader);
        if(it != shader_data_bindings.end())
            return false;

        auto insert_it = shader_data_bindings.insert({shader, ShaderDataBinding(render_world, 0)});
        render_world->NotifyNewShaderObjectData(shader, GetData(), &insert_it.first->second.index);

        return true;
    }

    void ObjectInstance::UpdateShaderDataBinding(const Shader* shader,
                                                 const hrs::block<vk::DeviceSize>& data_block,
                                                 vk::DeviceSize in_data_buffer_offset)
    {
        auto it = shader_data_bindings.find(shader);
        if(it == shader_data_bindings.end())
            return;

        it->second.render_world->NotifyUpdateShaderObjectData(shader,
                                                              GetData(),
                                                              it->second.index,
                                                              data_block,
                                                              in_data_buffer_offset);
    }

    void ObjectInstance::UpdateAllShaderDataBindings(const hrs::block<vk::DeviceSize>& data_block,
                                                     vk::DeviceSize in_data_buffer_offset)
    {
        auto data = GetData();
        for(auto& binding: shader_data_bindings)
        {
            binding.second.render_world->NotifyUpdateShaderObjectData(binding.first,
                                                                      data,
                                                                      binding.second.index,
                                                                      data_block,
                                                                      in_data_buffer_offset);
        }
    }

    std::optional<std::uint32_t>
    ObjectInstance::GetShaderDataIndex(const Shader* shader) const noexcept
    {
        auto it = shader_data_bindings.find(shader);
        if(it == shader_data_bindings.end())
            return {};

        return it->second.index;
    }

    std::vector<ObjectMeshBinding>& ObjectInstance::GetObjectMeshBindings() noexcept
    {
        return mesh_bindings;
    }

    const std::vector<ObjectMeshBinding>& ObjectInstance::GetObjectMeshBindings() const noexcept
    {
        return mesh_bindings;
    }

    void ObjectInstance::destroy()
    {
        mesh_bindings.clear();
        for(auto& binding: shader_data_bindings)
        {
            binding.second.render_world->NotifyRemoveShaderObjectData(binding.first,
                                                                      binding.second.index);
        }

        shader_data_bindings.clear();
    }
};
