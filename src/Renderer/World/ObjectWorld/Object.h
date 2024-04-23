#pragma once

#include <map>
#include <memory>
#include <span>
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "hrs/error.hpp"
#include "../../Allocator/BoundedResourceSize.hpp"
#include "ObjectInstance.h"

namespace FireLand
{
	class ObjectWorld;
	class Mesh;
	class ObjectInstance;
	class TransferChannel;

	class Object : public hrs::non_copyable
	{
	public:
		using ObjectInstanceContainer = std::map<const ObjectInstance *, std::unique_ptr<ObjectInstance>>;

		Object(ObjectWorld *_parent_object_world,
			   BoundedBufferSize &&_vertex_buffer,
			   BoundedBufferSize &&_index_buffer) noexcept;

		virtual ~Object();
		Object(Object &&obj) noexcept;
		Object & operator=(Object &obj) noexcept;

		ObjectWorld * GetParentObjectWorld() noexcept;
		const ObjectWorld * GetParentObjectWorld() const noexcept;

		void RemoveObjectInstance(const ObjectInstance *oi) noexcept;
		bool AddObjectInstance(std::unique_ptr<ObjectInstance> &&oi);

		ObjectInstanceContainer & GetObjectInstances() noexcept;
		const ObjectInstanceContainer & GetObjectInstances() const noexcept;

		hrs::error TransferMeshData(TransferChannel &channel,
									std::span<const std::byte> vertex_data,
									vk::DeviceSize vertex_buffer_offset,
									std::span<const std::uint32_t> index_data,
									std::uint32_t index_offset);

		virtual std::size_t GetMeshCount() const noexcept = 0;
		virtual const Mesh * GetMesh(std::size_t index) const noexcept = 0;

	private:
		void destroy();
	private:
		ObjectWorld *parent_object_world;

		ObjectInstanceContainer object_instances;

		BoundedBufferSize vertex_buffer;
		BoundedBufferSize index_buffer;
	};
};
