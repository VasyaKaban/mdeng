#include "ObjectMeshBinding.h"
#include "../RenderWorld/MaterialGroup.h"
#include "../RenderWorld/RenderGroup.h"
#include "../RenderWorld/RenderWorld.h"
#include "ObjectInstance.h"

namespace FireLand
{
    ObjectMeshBinding::ObjectMeshBinding(ObjectInstance* _parent_object_instance,
                                         const Mesh* _mesh) noexcept
        : parent_object_instance(_parent_object_instance),
          mesh(_mesh)
    {}

    ObjectMeshBinding::~ObjectMeshBinding()
    {
        destroy();
    }

    ObjectMeshBinding::ObjectMeshBinding(ObjectMeshBinding&& omb) noexcept
        : parent_object_instance(omb.parent_object_instance),
          mesh(omb.mesh),
          render_bindings(std::move(omb.render_bindings))
    {}

    ObjectMeshBinding& ObjectMeshBinding::operator=(ObjectMeshBinding&& omb) noexcept
    {
        destroy();

        parent_object_instance = omb.parent_object_instance;
        mesh = omb.mesh;
        render_bindings = std::move(omb.render_bindings);

        return *this;
    }

    ObjectInstance* ObjectMeshBinding::GetParentObjectInstance() noexcept
    {
        return parent_object_instance;
    }

    const ObjectInstance* ObjectMeshBinding::GetParentObjectInstance() const noexcept
    {
        return parent_object_instance;
    }

    const Mesh* ObjectMeshBinding::GetMesh() const noexcept
    {
        return mesh;
    }

    bool ObjectMeshBinding::HasRenderBinding(const RenderGroup* render_group) const noexcept
    {
        return render_bindings.find(render_group) != render_bindings.end();
    }

    void ObjectMeshBinding::RemoveRenderBinding(const RenderGroup* render_group) noexcept
    {
        auto it = render_bindings.find(render_group);
        if(it == render_bindings.end())
            return;

        it->second.render_world->NotifyRemoveRenderGroupInstance(it->second.render_group,
                                                                 it->second.index);

        render_bindings.erase(it);
    }

    bool ObjectMeshBinding::AddRenderBinding(const RenderGroup* render_group,
                                             RenderWorld* render_world)
    {
        if(render_group->GetMesh() != mesh)
            return false;

        auto shader = render_group->GetParentMaterialGroup()->GetParentShader();
        auto data_index = parent_object_instance->GetShaderDataIndex(shader);
        if(!data_index)
            return false;

        auto it = render_bindings.find(render_group);
        if(it != render_bindings.end())
            return false;

        auto insert_it =
            render_bindings.insert({render_group, RenderBinding(render_group, render_world, 0)});

        render_world->NotifyNewRenderGroupInstance(render_group,
                                                   *data_index,
                                                   &insert_it.first->second.index);

        return true;
    }

    void ObjectMeshBinding::destroy()
    {
        render_bindings.clear();
    }
};
