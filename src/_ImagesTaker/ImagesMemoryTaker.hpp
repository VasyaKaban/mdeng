#pragma once

#include <vector>
#include "../Allocator/Allocator.hpp"
#include "ImagesMemoryTypeNode.hpp"

namespace FireLand
{
	class ImagesMemoryTaker
	{
	public:

		ImagesMemoryTaker(Allocator *_allocator);
		~ImagesMemoryTaker() = default;
		ImagesMemoryTaker(const ImagesMemoryTaker &) = delete;
		ImagesMemoryTaker(ImagesMemoryTaker &&img_taker) noexcept;
		ImagesMemoryTaker & operator=(const ImagesMemoryTaker &) = delete;
		ImagesMemoryTaker & operator=(ImagesMemoryTaker &&img_taker) noexcept;

		constexpr const std::vector<ImagesMemoryTypeNode> & GetNodes() noexcept;
		constexpr ImagesMemoryTypeNode & operator[](std::size_t index) noexcept;
		constexpr const ImagesMemoryTypeNode & operator[](std::size_t index) const noexcept;
		constexpr Allocator * GetAllocator() noexcept;
		constexpr const Allocator * GetAllocator() const noexcept;

	private:
		Allocator *allocator;
		std::vector<ImagesMemoryTypeNode> memory_type_nodes;
	};

	ImagesMemoryTaker::ImagesMemoryTaker(Allocator *_allocator)
	{
		hrs::assert_true_debug(_allocator, "Allocator's pointer points to null!");
		hrs::assert_true_debug(_allocator->IsCreated(), "Allocator isn't created yet!");
		allocator = _allocator;
	}

	ImagesMemoryTaker::ImagesMemoryTaker(ImagesMemoryTaker &&img_taker) noexcept
		: allocator(img_taker.allocator), memory_type_nodes(std::move(img_taker.memory_type_nodes)) {}

	ImagesMemoryTaker & ImagesMemoryTaker::operator=(ImagesMemoryTaker &&img_taker) noexcept
	{
		this->~ImagesMemoryTaker();
		allocator = img_taker.allocator;
		memory_type_nodes = std::move(img_taker.memory_type_nodes);
		return *this;
	}

	constexpr const std::vector<ImagesMemoryTypeNode> & ImagesMemoryTaker::GetNodes() noexcept
	{
		return memory_type_nodes;
	}

	constexpr ImagesMemoryTypeNode & ImagesMemoryTaker::operator[](std::size_t index) noexcept
	{
		return memory_type_nodes[index];
	}

	constexpr const ImagesMemoryTypeNode & ImagesMemoryTaker::operator[](std::size_t index) const noexcept
	{
		return memory_type_nodes[index];
	}

	constexpr Allocator * ImagesMemoryTaker::GetAllocator() noexcept
	{
		return allocator;
	}

	constexpr const Allocator * ImagesMemoryTaker::GetAllocator() const noexcept
	{
		return allocator;
	}
};
