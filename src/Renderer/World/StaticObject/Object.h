#pragma once

#include "ReifiedObject.h"
#include "Mesh.h"
#include "ObjectInstance.h"
#include "../../../hrs/non_creatable.hpp"
#include <list>
#include <vector>
#include "../RenderWorld/RenderWorld.h"

namespace FireLand
{
	class World;
	enum class RenderGroupInstanceDataType;

	namespace StaticObject
	{
		class Object : public hrs::non_copyable
		{
		public:

			Object(const ReifiedObject *robject,
				   World *_parent_world,
				   const RenderGroupOptions &_options);
			~Object();

			void Destroy();

			World * GetParentWorld() noexcept;
			const World * GetParentWorld() const noexcept;

			const std::vector<Mesh> & GetMeshes() const noexcept;
			std::list<ObjectInstance> & GetObjectInstances() noexcept;
			const std::list<ObjectInstance> & GetObjectInstances() const noexcept;
			void EraseObjectInstance(std::list<ObjectInstance>::const_iterator it);

			std::list<ObjectInstance>::iterator
			AddObjectInstance(hrs::math::matrix_view<const float, 4, 4> _model_matrix);

			const RenderGroupOptions & GetRenderGroupOptions() const noexcept;
			void SetRenderGroupOptions(const RenderGroupOptions &_options) noexcept;

			vk::Buffer GetVertexBuffer() const noexcept;
			vk::Buffer GetIndexBuffer() const noexcept;

		private:
			World *parent_world;
			std::vector<Mesh> meshes;
			std::list<ObjectInstance> object_instances;

			//render_world groups create info!
			RenderGroupOptions options;
		};
	};
};
