#pragma once

#include <string>
#include <map>
#include <memory>
#include "hrs/non_creatable.hpp"
#include "../../Allocator/MemoryType.h"
#include "../../Allocator/BoundedResourceSize.hpp"
#include "Object.h"

namespace FireLand
{
	class Object;
	class Device;

	struct ObjectsKeyComparator
	{
		using is_transparent = void;

		bool operator()(const std::string &str1, const std::string &str2) const noexcept
		{
			return str1 < str2;
		}

		bool operator()(const std::string &str, std::string_view str_view) const noexcept
		{
			return str < str_view;
		}

		bool operator()(std::string_view str_view, const std::string &str) const noexcept
		{
			return str_view < str;
		}

		bool operator()(std::string_view str_view1, std::string_view str_view2) const noexcept
		{
			return str_view1 < str_view2;
		}
	};

	class ObjectPayload : public hrs::non_copyable
	{
	public:
		ObjectPayload(Allocator *_allocator,
					  BoundedBufferSize &&_vertex_buffer,
					  BoundedBufferSize &&_index_buffer) noexcept
			: allocator(_allocator),
			  vertex_buffer(std::move(_vertex_buffer)),
			  index_buffer(std::move(_index_buffer)) {}

		~ObjectPayload()
		{
			if(*this)
			{
				allocator->Release(vertex_buffer.bounded_buffer, MemoryPoolOnEmptyPolicy::Free);
				allocator->Release(index_buffer.bounded_buffer, MemoryPoolOnEmptyPolicy::Free);

				vertex_buffer = {};
				index_buffer = {};
			}
		}

		ObjectPayload(ObjectPayload &&) = default;
		ObjectPayload & operator=(ObjectPayload &&) = default;

		std::pair<BoundedBufferSize, BoundedBufferSize> Release() noexcept
		{
			return {std::move(vertex_buffer), std::move(index_buffer)};
		}

		explicit operator bool() const noexcept
		{
			return allocator;
		}

		bool HasVertexBuffer() const noexcept
		{
			return vertex_buffer.bounded_buffer.IsCreated();
		}

		bool HasIndexBuffer() const noexcept
		{
			return index_buffer.bounded_buffer.IsCreated();
		}

	private:
		Allocator *allocator;
		BoundedBufferSize vertex_buffer;
		BoundedBufferSize index_buffer;
	};

	class ObjectWorld : public hrs::non_copyable
	{
	public:
		using ObjectContainer = std::map<std::string, std::unique_ptr<Object>, ObjectsKeyComparator>;


		ObjectWorld(Device *_parent_device,
					const std::function<NewPoolSizeCalculator> &_calc);

		~ObjectWorld();
		ObjectWorld(ObjectWorld &&ow) noexcept;
		ObjectWorld & operator=(ObjectWorld &&ow) noexcept;

		hrs::expected<ObjectPayload, hrs::error>
		AcquireObjectPayload(std::size_t vertex_data_size,
							 std::size_t index_count,
							 vk::DeviceSize vertex_data_alignment);

		//AcquireObjectPayload -> Create Object impl -> Call TransferMeshData -> Call AddObject
		bool AddObject(std::string_view name, std::unique_ptr<Object> &&object);

		bool HasObject(std::string_view name) const noexcept;
		void RemoveObject(std::string_view name);

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;

		const std::function<NewPoolSizeCalculator> & GetNewPoolSizeCalculator() const noexcept;

		ObjectContainer & GetObjects() noexcept;
		const ObjectContainer & GetObjects() const noexcept;

	private:

		void destroy();

		hrs::expected<BoundedBufferSize, hrs::error> allocate_buffer(vk::BufferUsageFlags usage,
																	 vk::DeviceSize size,
																	 vk::DeviceSize alignment);

	private:
		Device *parent_device;

		std::function<NewPoolSizeCalculator> calc;
		ObjectContainer objects;
	};
};
