#include "Object.h"
#include "../../TransferChannel/TransferChannel.h"

namespace FireLand
{
	Object::Object(ObjectWorld *_parent_object_world,
				   BoundedBufferSize &&_vertex_buffer,
				   BoundedBufferSize &&_index_buffer) noexcept
		: parent_object_world(_parent_object_world),
		  vertex_buffer(std::move(_vertex_buffer)),
		  index_buffer(std::move(_index_buffer)) {}

	Object::~Object()
	{
		destroy();
	}

	Object::Object(Object &&obj) noexcept
		: parent_object_world(obj.parent_object_world),
		  object_instances(std::move(obj.object_instances)),
		  vertex_buffer(std::move(obj.vertex_buffer)),
		  index_buffer(std::move(obj.index_buffer)) {}

	Object & Object::operator=(Object &obj) noexcept
	{
		destroy();

		parent_object_world = obj.parent_object_world;
		object_instances = std::move(obj.object_instances);
		vertex_buffer = std::move(obj.vertex_buffer);
		index_buffer = std::move(obj.index_buffer);

		return *this;
	}

	ObjectWorld * Object::GetParentObjectWorld() noexcept
	{
		return parent_object_world;
	}

	const ObjectWorld * Object::GetParentObjectWorld() const noexcept
	{
		return parent_object_world;
	}

	void Object::RemoveObjectInstance(const ObjectInstance *oi) noexcept
	{
		object_instances.erase(oi);
	}

	bool Object::AddObjectInstance(std::unique_ptr<ObjectInstance> &&oi)
	{
		auto it = object_instances.insert({oi.get(), std::move(oi)});
		return it.second;
	}

	Object::ObjectInstanceContainer & Object::GetObjectInstances() noexcept
	{
		return object_instances;
	}

	const Object::ObjectInstanceContainer & Object::GetObjectInstances() const noexcept
	{
		return object_instances;
	}

	hrs::error Object::TransferMeshData(TransferChannel &channel,
										std::span<const std::byte> vertex_data,
										vk::DeviceSize vertex_buffer_offset,
										std::span<const std::uint32_t> index_data,
										std::uint32_t index_offset)
	{
		vk::DeviceSize vertex_min_size = std::min(vertex_data.size(), vertex_buffer.size);
		if(vertex_min_size != 0)
		{
			if(vertex_buffer_offset + vertex_min_size <= vertex_buffer.size)
			{
				Data data(vertex_data.data());
				TransferBufferOpRegion region(hrs::block<vk::DeviceSize>(vertex_min_size, 0),
											  vertex_buffer_offset,
											  0);
				auto exp = channel.CopyBuffer(vertex_buffer.bounded_buffer.buffer,
											  {&data, 1},
											  {&region, 1});
				if(!exp)
					return exp.error();
			}
		}

		vk::DeviceSize index_min_size = std::min(index_data.size() * sizeof(std::uint32_t),
												 index_buffer.size);
		if(index_min_size != 0)
		{
			if(index_offset * sizeof(std::uint32_t) + index_min_size <= vertex_buffer.size)
			{
				Data data(index_data.data());
				TransferBufferOpRegion region(hrs::block<vk::DeviceSize>(index_min_size, 0),
											  index_offset * sizeof(std::uint32_t),
											  0);
				auto exp = channel.CopyBuffer(index_buffer.bounded_buffer.buffer,
											  {&data, 1},
											  {&region, 1});

				if(!exp)
					return exp.error();
			}
		}

		return vk::Result::eSuccess;
	}

	void Object::destroy()
	{
		object_instances.clear();
	}
};
