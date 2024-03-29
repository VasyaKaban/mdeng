#include "MeshMaterialBinding.h"
#include "../RenderWorld/RenderWorld.h"
#include "ObjectInstance.h"
#include "Object.h"
#include "../RenderWorld/Material.h"
#include "Mesh.h"
#include "../World.h"

namespace FireLand
{
	namespace StaticObject
	{
		MeshMaterialBinding::MeshMaterialBinding(ObjectInstance *_parent_object_instance,
												 const Mesh *_mesh,
												 std::span<Material *> _materials)
			: parent_object_instance(_parent_object_instance),
			  mesh(_mesh)
		{
			for(auto &mtl : _materials)
			{
				RenderGroup *render_group = get_or_create_render_group(mtl);
				add_new_material(mtl, render_group);
			}
		}

		MeshMaterialBinding::~MeshMaterialBinding()
		{
			Destroy();
		}

		void MeshMaterialBinding::Destroy()
		{
			RenderWorld *render_world =
				parent_object_instance->GetParentObject()->GetParentWorld()->GetRenderWorld();

			for(auto &binding : render_group_bindings)
				render_world->NotifyRemoveRenderGroupInstance(binding.first, binding.second);

			render_group_bindings.clear();
		}

		ObjectInstance * MeshMaterialBinding::GetParentObjectInstance() noexcept
		{
			return parent_object_instance;
		}

		const ObjectInstance * MeshMaterialBinding::GetParentObjectInstance() const noexcept
		{
			return parent_object_instance;
		}

		const Mesh * MeshMaterialBinding::GetMesh() const noexcept
		{
			return mesh;
		}

		const std::map<RenderGroup *, std::uint32_t> & MeshMaterialBinding::GetRenderGroupBindings() const noexcept
		{
			return render_group_bindings;
		}

		void MeshMaterialBinding::AddMaterial(Material *material)
		{
			RenderGroup *render_group = get_or_create_render_group(material);
			auto it = render_group_bindings.find(render_group);
			if(it != render_group_bindings.end())
				return;

			add_new_material(material, render_group);
		}

		void MeshMaterialBinding::RemoveMaterial(const Material *material)
		{
			RenderWorld *render_world =
				parent_object_instance->GetParentObject()->GetParentWorld()->GetRenderWorld();

			auto [it, exists] = find_render_group(material);
			if(!exists)
				return;

			render_world->NotifyRemoveRenderGroupInstance(it->first, it->second);
			render_group_bindings.erase(it);
		}

		RenderGroup * MeshMaterialBinding::get_or_create_render_group(const Material *material)
		{
			Object *parent_object = parent_object_instance->GetParentObject();
			RenderWorld *render_world = parent_object->GetParentWorld()->GetRenderWorld();
			RenderGroup *render_group = render_world->GetRenderGroup(material, mesh);
			if(render_group)
				return render_group;

			return render_world->NotifyNewRenderGroup(material,
													  mesh,
													  parent_object->GetRenderGroupOptions());
		}

		void MeshMaterialBinding::add_new_material(Material *material, RenderGroup *render_group)
		{
			RenderWorld *render_world =
				parent_object_instance->GetParentObject()->GetParentWorld()->GetRenderWorld();

			std::size_t offset = parent_object_instance->AddShaderBinding(material->GetShader());
			std::uint32_t index = render_world->NotifyNewRenderGroupInstance(render_group, offset);
			render_group_bindings.insert({render_group, index});
		}

		std::pair<std::map<RenderGroup *, std::uint32_t>::const_iterator, bool>
		MeshMaterialBinding::find_render_group(const Material *material) const noexcept
		{
			RenderWorld *render_world =
				parent_object_instance->GetParentObject()->GetParentWorld()->GetRenderWorld();

			RenderGroup *render_group = render_world->GetRenderGroup(material, mesh);
			if(!render_group)
				return {{}, false};

			auto it = render_group_bindings.find(render_group);
			if(it == render_group_bindings.end())
				return {{}, false};

			return {it, true};
		}
	};
};
