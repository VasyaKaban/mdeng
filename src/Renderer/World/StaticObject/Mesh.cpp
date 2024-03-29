#include "Mesh.h"
#include "Object.h"

namespace FireLand
{
	namespace StaticObject
	{
		Mesh::Mesh(vk::DeviceSize _index_buffer_offset,
				   std::uint32_t _count,
				   Object *_parent_object) noexcept
			: index_buffer_offset(_index_buffer_offset),
			  count(_count),
			  parent_object(_parent_object) {}

		void Mesh::Bind(vk::CommandBuffer command_buffer,
						vk::Buffer prev_vertex_buffer,
						vk::Buffer prev_index_buffer) const noexcept
		{
			vk::Buffer vertex_buffer = parent_object->GetVertexBuffer();
			vk::Buffer index_buffer = parent_object->GetIndexBuffer();

			if(vertex_buffer != prev_vertex_buffer)
				command_buffer.bindVertexBuffers(0, vertex_buffer, {0});

			if(index_buffer != prev_index_buffer)
				command_buffer.bindIndexBuffer(index_buffer, 0, vk::IndexType::eUint32);
		}

		void Mesh::Draw(vk::CommandBuffer command_buffer, std::uint32_t instance_count) const noexcept
		{
			command_buffer.drawIndexed(count, instance_count, index_buffer_offset / sizeof(std::uint32_t), 0, 0);
		}

		std::pair<vk::Buffer, vk::DeviceSize> Mesh::GetVertexBuffer() const noexcept
		{
			return {parent_object->GetVertexBuffer(), 0};
		}

		std::pair<vk::Buffer, vk::DeviceSize> Mesh::GetIndexBuffer() const noexcept
		{
			return {parent_object->GetIndexBuffer(), index_buffer_offset};
		}

		std::uint32_t Mesh::GetCount() const noexcept
		{
			return count;
		}

		Object * Mesh::GetParentObject() noexcept
		{
			return parent_object;
		}

		const Object * Mesh::GetParentObject() const noexcept
		{
			return parent_object;
		}
	};
};
