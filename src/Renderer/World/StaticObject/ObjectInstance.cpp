#include "ObjectInstance.h"
#include "Object.h"
#include "../World.h"
#include "../RenderWorld/RenderWorld.h"

namespace FireLand
{
	namespace StaticObject
	{
		ObjectInstance::ObjectInstance(Object *_parent_object,
									   hrs::math::matrix_view<const float, 4, 4> _model_matrix)
			: parent_object(_parent_object),
			  model_matrix(_model_matrix)
		{
			const auto &meshes = parent_object->GetMeshes();
			mesh_material_bindings.reserve(meshes.size());
			for(const auto &mesh : meshes)
				mesh_material_bindings.push_back(MeshMaterialBinding(this, &mesh));
		}

		ObjectInstance::~ObjectInstance()
		{
			Destroy();
		}

		void ObjectInstance::Destroy()
		{
			mesh_material_bindings.clear();
			RenderWorld *render_world = parent_object->GetParentWorld()->GetRenderWorld();
			for(auto &shader_binding : shader_bindings_data)
				render_world->NotifyRemoveShaderObjectData(shader_binding.first, shader_binding.second);

			shader_bindings_data.clear();
		}

		Object * ObjectInstance::GetParentObject() noexcept
		{
			return parent_object;
		}

		const Object * ObjectInstance::GetParentObject() const noexcept
		{
			return parent_object;
		}

		const hrs::math::glsl::std430::mat4x4 & ObjectInstance::GetModelMatrix() const noexcept
		{
			return model_matrix;
		}

		std::span<const std::byte> ObjectInstance::GetModelMatrixData() const noexcept
		{
			return std::span<const std::byte>(reinterpret_cast<const std::byte *>(model_matrix.data),
											  sizeof(model_matrix));
		}

		void ObjectInstance::UpdateModelMatrix(hrs::math::matrix_view<const float, 4, 4> _model_matrix)
		{
			RenderWorld *render_world = parent_object->GetParentWorld()->GetRenderWorld();
			model_matrix = _model_matrix;
			auto data = GetModelMatrixData();
			for(auto &shader_binding : shader_bindings_data)
			{
				render_world->NotifyUpdateShaderObjectData(shader_binding.first,
														   shader_binding.second,
														   0,
														   data);
			}
		}

		std::vector<MeshMaterialBinding> & ObjectInstance::GetMeshMaterialBindings() noexcept
		{
			return mesh_material_bindings;
		}

		const std::vector<MeshMaterialBinding> & ObjectInstance::GetMeshMaterialBindings() const noexcept
		{
			return mesh_material_bindings;
		}

		const ObjectInstance::ShaderBindings & ObjectInstance::GetShaderBindingsData() const noexcept
		{
			return shader_bindings_data;
		}

		std::size_t ObjectInstance::AddShaderBinding(Shader *shader)
		{
			auto it = shader_bindings_data.find(shader);
			if(it != shader_bindings_data.end())
				return it->second;

			RenderWorld *render_world = parent_object->GetParentWorld()->GetRenderWorld();
			std::size_t offset = render_world->NotifyNewShaderObjectData(shader,
																		 0,
																		 GetModelMatrixData());
			shader_bindings_data.insert({shader, offset});
			return offset;
		}
	};
};
