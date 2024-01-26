#pragma once

#include <list>
#include "../Allocator/Allocator.hpp"
#include "ImagesFreeBlockBuffer.hpp"

namespace FireLand
{
	class ImagesMemoryTypeNode
	{
	public:

		using BufferHandle = std::list<ImagesFreeBlockBuffer>::iterator;
		using ConstBufferHandle = std::list<ImagesFreeBlockBuffer>::const_iterator;

		ImagesMemoryTypeNode(Allocator *_allocator, const MemoryType &_memory_type);
		~ImagesMemoryTypeNode() = default;
		ImagesMemoryTypeNode(const ImagesMemoryTypeNode &) = delete;
		ImagesMemoryTypeNode(ImagesMemoryTypeNode &&node) noexcept;
		ImagesMemoryTypeNode & operator=(const ImagesMemoryTypeNode &) = delete;
		ImagesMemoryTypeNode & operator=(ImagesMemoryTypeNode &&node) noexcept;

		constexpr const MemoryType & GetMemoryType() const noexcept;
		constexpr const std::list<ImagesFreeBlockBuffer> & GetBuffers() const noexcept;

		hrs::expected<BufferHandle, vk::Result>
		Allocate(vk::DeviceSize size, vk::DeviceSize alignment, bool map_memory);

		hrs::expected<BufferHandle, vk::Result>
		Reallocate(BufferHandle handle, vk::DeviceSize size, vk::DeviceSize alignment, bool map_memory);

		BufferHandle operator[](std::size_t index) noexcept;
		ConstBufferHandle operator[](std::size_t index) const noexcept;
		void Free(BufferHandle handle) noexcept;

		constexpr bool IsEmpty() const noexcept;
		constexpr bool IsMappable() const noexcept;

		constexpr Allocator * GetAllocator() noexcept;
		constexpr const Allocator * GetAllocator() const noexcept;

	private:

		void debug_assert_handle_is_part_of_this(BufferHandle handle) noexcept;

		Allocator *allocator;
		MemoryType memory_type;
		std::list<ImagesFreeBlockBuffer> buffers;
	};

	inline ImagesMemoryTypeNode::ImagesMemoryTypeNode(Allocator *_allocator, const MemoryType &_memory_type)
	{
		hrs::assert_true_debug(_allocator, "Allocator's pointer points to null!");
		hrs::assert_true_debug(_allocator->IsCreated(), "Allocator isn't created yet!");
		allocator = _allocator;
		memory_type = _memory_type;
	}

	inline ImagesMemoryTypeNode::ImagesMemoryTypeNode(ImagesMemoryTypeNode &&node) noexcept
		: allocator(node.allocator), memory_type(node.memory_type), buffers(std::move(node.buffers)) {}

	inline ImagesMemoryTypeNode & ImagesMemoryTypeNode::operator=(ImagesMemoryTypeNode &&node) noexcept
	{
		this->~ImagesMemoryTypeNode();
		allocator = node.allocator;
		memory_type = node.memory_type;
		buffers = std::move(node.buffers);
		return *this;
	}

	constexpr const MemoryType & ImagesMemoryTypeNode::GetMemoryType() const noexcept
	{
		return memory_type;
	}

	constexpr const std::list<ImagesFreeBlockBuffer> & ImagesMemoryTypeNode::GetBuffers() const noexcept
	{
		return buffers;
	}

	inline hrs::expected<ImagesMemoryTypeNode::BufferHandle, vk::Result>
	ImagesMemoryTypeNode::Allocate(vk::DeviceSize size, vk::DeviceSize alignment, bool map_memory)
	{
		hrs::assert_true_debug(((map_memory == true && !memory_type.IsMappable()) ? false : true),
							   "Memory cannot be mapped because memory type isn't mappable!");
		hrs::assert_true_debug(hrs::is_multiple_of(size, alignment),
							   "Size={} isn't multiple of alignment={}!",
							   size, alignment);

		auto alloc_result = allocator->Allocate(memory_type, size, map_memory);
		if(!alloc_result)
			return alloc_result.error().result;

		buffers.push_back(ImagesFreeBlockBuffer(allocator->GetDevice(),
												std::move(alloc_result.value()),
												alignment));
		return std::prev(buffers.end());
	}

	inline hrs::expected<ImagesMemoryTypeNode::BufferHandle, vk::Result>
	ImagesMemoryTypeNode::Reallocate(BufferHandle handle,
									 vk::DeviceSize size,
									 vk::DeviceSize alignment,
									 bool map_memory)
	{
		debug_assert_handle_is_part_of_this(handle);

		buffers.erase(handle);
		return Allocate(size, alignment, map_memory);
	}

	inline ImagesMemoryTypeNode::BufferHandle ImagesMemoryTypeNode::operator[](std::size_t index) noexcept
	{
		if(index >= buffers.size())
			return buffers.end();

		return std::next(buffers.begin(), index);
	}

	inline ImagesMemoryTypeNode::ConstBufferHandle
	ImagesMemoryTypeNode::operator[](std::size_t index) const noexcept
	{
		if(index >= buffers.size())
			return buffers.end();

		return std::next(buffers.begin(), index);
	}

	inline void ImagesMemoryTypeNode::Free(BufferHandle handle) noexcept
	{
		void debug_assert_handle_is_part_of_this(BufferHandle handle);

		buffers.erase(handle);
	}

	constexpr bool ImagesMemoryTypeNode::IsEmpty() const noexcept
	{
		return buffers.empty();
	}

	constexpr bool ImagesMemoryTypeNode::IsMappable() const noexcept
	{
		return memory_type.IsMappable();
	}

	constexpr Allocator * ImagesMemoryTypeNode::GetAllocator() noexcept
	{
		return allocator;
	}

	constexpr const Allocator * ImagesMemoryTypeNode::GetAllocator() const noexcept
	{
		return allocator;
	}

	void ImagesMemoryTypeNode::debug_assert_handle_is_part_of_this(BufferHandle handle) noexcept
	{
	#ifndef NDEBUG
		bool found = false;
		for(auto it = buffers.begin(); it != buffers.end(); it++)
			if(handle == it)
			{
				found = true;
				break;
			}

		hrs::assert_true_debug(found, "Passed buffer handle is not a part of this node!");
	#endif
	}
};
