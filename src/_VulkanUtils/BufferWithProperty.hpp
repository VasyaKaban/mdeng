/**
 * @file Represents BufferWithProperty class
 */

#pragma once

#include "../Allocator/Allocator.hpp"

namespace FireLand
{
	/**
	 * @brief The BufferWithProperty class
	 *
	 * Helpful structure for buffer and it's property flags
	 */
	struct BufferWithProperty
	{
		Buffer buffer_memory;///<buffer
		vk::MemoryPropertyFlags prop_flags;///<buffer's property flags

		BufferWithProperty() = default;
		BufferWithProperty(BufferWithProperty &&buf) noexcept = default;
		BufferWithProperty(Buffer &&_buffer_memory, vk::MemoryPropertyFlags _prop_flags)
			: buffer_memory(std::move(_buffer_memory)), prop_flags(_prop_flags) {}

		constexpr BufferWithProperty & operator=(BufferWithProperty &&buf) noexcept = default;
	};
};
