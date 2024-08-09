#pragma once

#include "../Vulkan/VulkanInclude.h"
#include <bit>
#include "hrs/non_creatable.hpp"

namespace FireLand
{
	class DeviceLoader;

	class Memory
		: public hrs::non_copyable,
		  public hrs::non_move_assignable
	{
	public:
		Memory(VkDeviceMemory _memory = VK_NULL_HANDLE,
			   VkDeviceSize _size = 0,
			   std::byte *_map_ptr = nullptr) noexcept;
		~Memory() = default;
		Memory(Memory &&mem) noexcept;

		void Free(VkDevice device, const DeviceLoader &dl, const VkAllocationCallbacks *alc) noexcept;

		bool IsAllocated() const noexcept;
		bool IsMapped() const noexcept;

		VkDeviceMemory GetDeviceMemory() const noexcept;
		VkDeviceSize GetSize() const noexcept;

		VkResult MapMemory(VkDevice device, const DeviceLoader &dl) noexcept;

		template<typename T = std::byte>
		T * GetMapPtr() noexcept
		{
			return reinterpret_cast<T *>(map_ptr);
		}

		template<typename T = std::byte>
		const T * GetMapPtr() const noexcept
		{
			return reinterpret_cast<const T *>(map_ptr);
		}

	private:
		VkDeviceMemory memory;
		VkDeviceSize size;
		std::byte *map_ptr;
	};
};
