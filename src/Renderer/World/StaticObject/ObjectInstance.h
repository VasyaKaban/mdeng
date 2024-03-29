#pragma once

#include <map>
#include "Mesh.h"
#include "../../../hrs/math/matrix.hpp"
#include "../../../hrs/math/matrix_view.hpp"
#include "MeshMaterialBinding.h"

namespace FireLand
{
	class Object;
	class Shader;

	namespace StaticObject
	{
		struct ShaderMapTransparentCompareLess
		{
			using is_transparent = void;

			bool operator()(const Shader *const sh0, const Shader *const sh1) const noexcept
			{
				return sh0 < sh1;
			}
		};

		class ObjectInstance
		{
		public:

			using ShaderBindings = std::map<Shader *, std::size_t, ShaderMapTransparentCompareLess>;

			ObjectInstance(Object *_parent_object, hrs::math::matrix_view<const float, 4, 4> _model_matrix);
			~ObjectInstance();

			void Destroy();

			Object * GetParentObject() noexcept;
			const Object * GetParentObject() const noexcept;

			const hrs::math::glsl::std430::mat4x4 & GetModelMatrix() const noexcept;
			std::span<const std::byte> GetModelMatrixData() const noexcept;
			void UpdateModelMatrix(hrs::math::matrix_view<const float, 4, 4> _model_matrix);

			std::vector<MeshMaterialBinding> & GetMeshMaterialBindings() noexcept;
			const std::vector<MeshMaterialBinding> & GetMeshMaterialBindings() const noexcept;

			const ShaderBindings & GetShaderBindingsData() const noexcept;

			std::size_t AddShaderBinding(Shader *shader);

		private:
			Object *parent_object;
			hrs::math::glsl::std430::mat4x4 model_matrix;
			std::vector<MeshMaterialBinding> mesh_material_bindings;
			ShaderBindings shader_bindings_data;
		};
	};
};

