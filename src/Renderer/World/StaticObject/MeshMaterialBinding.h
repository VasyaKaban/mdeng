#pragma once

#include "Mesh.h"
#include "../../../hrs/math/matrix_common.hpp"
#include <map>

namespace FireLand
{
	class RenderGroup;
	class Material;

	namespace StaticObject
	{
		class ObjectInstance;

		class MeshMaterialBinding
		{
		public:
			MeshMaterialBinding(ObjectInstance *_parent_object_instance,
								const Mesh *_mesh,
								std::span<Material *> _materials = {});

			~MeshMaterialBinding();

			void Destroy();

			ObjectInstance * GetParentObjectInstance() noexcept;
			const ObjectInstance * GetParentObjectInstance() const noexcept;
			const Mesh * GetMesh() const noexcept;
			const std::map<RenderGroup *, std::uint32_t> & GetRenderGroupBindings() const noexcept;

			void AddMaterial(Material *material);
			void RemoveMaterial(const Material *material);

		private:

			RenderGroup * get_or_create_render_group(const Material *material);
			void add_new_material(Material *material, RenderGroup *render_group);

			std::pair<std::map<RenderGroup *, std::uint32_t>::const_iterator, bool>
			find_render_group(const Material *material) const noexcept;

		private:
			ObjectInstance *parent_object_instance;
			const Mesh *mesh;
			//render group and offset!!!
			std::map<RenderGroup *, std::uint32_t> render_group_bindings;
		};
	};
};
