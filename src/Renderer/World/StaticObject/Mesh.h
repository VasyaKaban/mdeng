#pragma once

#include "../RenderWorld/Mesh.h"
#include <cstdint>

namespace FireLand
{
	namespace StaticObject
	{
		class Object;

		class Mesh : public FireLand::Mesh
		{
		public:
			Mesh(vk::DeviceSize _index_buffer_offset,
				 std::uint32_t _count,
				 Object *_parent_object) noexcept;

			Mesh(const Mesh &) = default;
			Mesh & operator=(const Mesh &) = default;

			virtual void Bind(vk::CommandBuffer command_buffer,
							  vk::Buffer prev_vertex_buffer,
							  vk::Buffer prev_index_buffer) const noexcept override;

			virtual void Draw(vk::CommandBuffer command_buffer, std::uint32_t instance_count) const noexcept override;
			virtual std::pair<vk::Buffer, vk::DeviceSize> GetVertexBuffer() const noexcept override;
			virtual std::pair<vk::Buffer, vk::DeviceSize> GetIndexBuffer() const noexcept override;
			virtual std::uint32_t GetCount() const noexcept override;

			Object * GetParentObject() noexcept;
			const Object * GetParentObject() const noexcept;

		private:
			Object *parent_object;
			std::uint32_t count;
			vk::DeviceSize index_buffer_offset;
		};
	};
};
