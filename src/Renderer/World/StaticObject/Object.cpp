#include "Object.h"
#include "Mesh.h"
#include "../RenderWorld/RenderWorld.h"

namespace FireLand
{
	namespace StaticObject
	{
		Object::Object(const ReifiedObject *robject,
					   World *_parent_world,
					   const RenderGroupOptions &_options)
			: parent_world(_parent_world),
			  options(_options)
		{
			meshes.reserve(robject->GetMeshCount());
			vk::DeviceSize vertex_buffer_size = robject->GetMeshVertexData().size();
			vk::DeviceSize index_buffer_size = robject->GetMeshIndexData().size() * sizeof(std::uint32_t);
			for(std::size_t i = 0; i < robject->GetMeshCount(); i++)
			{
				meshes.push_back(Mesh(robject->GetMeshIndexDataOffset(i),
									  robject->GetMeshIndicesCount(i),
									  this));
			}
#error TBA!!!
#error allocate buffers and copy
		}

		Object::~Object()
		{
			Destroy();
		}

		void Object::Destroy()
		{
			object_instances.clear();
			meshes.clear();
		}

		World * Object::GetParentWorld() noexcept
		{
			return parent_world;
		}

		const World * Object::GetParentWorld() const noexcept
		{
			return parent_world;
		}

		const std::vector<Mesh> & Object::GetMeshes() const noexcept
		{
			return meshes;
		}

		std::list<ObjectInstance> & Object::GetObjectInstances() noexcept
		{
			return object_instances;
		}

		const std::list<ObjectInstance> & Object::GetObjectInstances() const noexcept
		{
			return object_instances;
		}

		void Object::EraseObjectInstance(std::list<ObjectInstance>::const_iterator it)
		{
			object_instances.erase(it);
		}

		std::list<ObjectInstance>::iterator
		Object::AddObjectInstance(hrs::math::matrix_view<const float, 4, 4> _model_matrix)
		{
			object_instances.push_back(ObjectInstance(this, _model_matrix));
			return std::prev(object_instances.end());
		}

		const RenderGroupOptions & Object::GetRenderGroupOptions() const noexcept
		{
			return options;
		}

		void Object::SetRenderGroupOptions(const RenderGroupOptions &_options) noexcept
		{
			options = _options;
		}

		vk::Buffer Object::GetVertexBuffer() const noexcept
		{
#error TBA!!!
		}

		vk::Buffer Object::GetIndexBuffer() const noexcept
		{

		}
	};
};
