/**
 * @file
 *
 * Represents Memory class of vulkan allocator
 */

#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "../hrs/debug.hpp"

namespace FireLand
{
	/**
	 * @brief The Memory class
	 *
	 * Encapsulates memory, size and pointer to mapped memory
	 */
	struct Memory
	{
		vk::DeviceMemory device_memory;///<allocated memory
		vk::DeviceSize size;///<size in bytes
		std::byte *map_ptr;///<pointer to mapped memory

		constexpr Memory(vk::DeviceMemory _device_memory = VK_NULL_HANDLE,
						 vk::DeviceSize _size = 0,
						 std::byte *_map_ptr = nullptr)
			: device_memory(_device_memory), size(_size), map_ptr(_map_ptr) {}

		constexpr Memory(const Memory &) noexcept = default;
		constexpr Memory(Memory &&mem) noexcept
			: device_memory(mem.device_memory), size(mem.size), map_ptr(mem.map_ptr)
		{
			mem.device_memory = VK_NULL_HANDLE;
			mem.size = 0;
			mem.map_ptr = nullptr;
		}

		constexpr Memory & operator=(const Memory &) noexcept = default;
		constexpr Memory & operator=(Memory &&mem) noexcept
		{
			device_memory = mem.device_memory;
			size = mem.size;
			map_ptr = mem.map_ptr;
			mem.device_memory = VK_NULL_HANDLE;
			mem.size = 0;
			mem.map_ptr = nullptr;
			return *this;
		}

		/**
		 * @brief operator bool
		 *
		 * Checks whether object is created or not.
		 * Just checks that memory isn't VK_NULL_HANDLE
		 */
		constexpr explicit operator bool() const noexcept
		{
			return device_memory;
		}

		/**
		 * @brief IsMapped
		 * @return true if mapped pointer isn't nullptr
		 */
		constexpr bool IsMapped() const noexcept
		{
			return map_ptr;
		}

		/**
		 * @brief Free
		 * @param device the device that created this buffer
		 *
		 * Frees memory
		 * @warning Aborts if device isn't created!
		 */
		void Free(vk::Device device)
		{
			if(!*this)
				return;

			hrs::assert_true_debug(device, "Device isn't created yet!");
			if(map_ptr)
				device.unmapMemory(device_memory);

			device.freeMemory(device_memory);

			device_memory = VK_NULL_HANDLE;
			size = 0;
			map_ptr = nullptr;
		}

		template<typename T>
		T * Cast() noexcept
		{
			return reinterpret_cast<T *>(map_ptr);
		}
	};
};
