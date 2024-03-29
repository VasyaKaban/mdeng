#pragma once

#include "../../Vulkan/VulkanInclude.hpp"

namespace FireLand
{
	struct Memory
	{
		vk::DeviceMemory memory;
		vk::DeviceSize size;
		std::byte *map_ptr;

		Memory(vk::DeviceMemory _memory = {}, vk::DeviceSize _size = {}, std::byte *_map_ptr = {}) noexcept;
		~Memory() = default;
		Memory(const Memory &) = default;
		Memory(Memory &&mem) noexcept;
		Memory & operator=(const Memory &) = default;
		Memory & operator=(Memory &&mem) noexcept;

		void Free(vk::Device device) noexcept;

		bool IsAllocated() const noexcept;
		bool IsMapped() const noexcept;

		vk::DeviceMemory GetDeviceMemory() const noexcept;
		vk::DeviceSize GetSize() const noexcept;

		[[nodiscard]] vk::Result MapMemory(vk::Device device) noexcept;

		template<typename T = std::byte>
		constexpr T * GetMapPtr() noexcept
		{
			return reinterpret_cast<T *>(map_ptr);
		}

		template<typename T = std::byte>
		constexpr const T * GetMapPtr() const noexcept
		{
			return reinterpret_cast<const T *>(map_ptr);
		}
	};
};
