#include "Memory.h"
#include <utility>
#include "hrs/debug.hpp"

namespace FireLand
{
	Memory::Memory(vk::DeviceMemory _memory, vk::DeviceSize _size, std::byte *_map_ptr) noexcept
		: memory(_memory),
		  size(_size),
		  map_ptr(_map_ptr) {}

	Memory::Memory(Memory &&mem) noexcept
		: memory(std::exchange(mem.memory, VK_NULL_HANDLE)),
		  size(std::exchange(mem.size, 0)),
		  map_ptr(std::exchange(mem.map_ptr, nullptr)) {}

	Memory & Memory::operator=(Memory &&mem) noexcept
	{
		memory = std::exchange(mem.memory, VK_NULL_HANDLE);
		size = std::exchange(mem.size, 0);
		map_ptr = std::exchange(mem.map_ptr, nullptr);

		return *this;
	}

	void Memory::Free(vk::Device device) noexcept
	{
		hrs::assert_true_debug(device, "Device isn't created yet!");
		if(IsMapped())
		{
			device.unmapMemory(memory);
			map_ptr = nullptr;
		}

		if(IsAllocated())
		{
			device.free(memory);
			size = 0;
		}
	}

	bool Memory::IsAllocated() const noexcept
	{
		return memory;
	}

	bool Memory::IsMapped() const noexcept
	{
		return map_ptr;
	}

	vk::DeviceMemory Memory::GetDeviceMemory() const noexcept
	{
		return memory;
	}

	vk::DeviceSize Memory::GetSize() const noexcept
	{
		return size;
	}

	vk::Result Memory::MapMemory(vk::Device device) noexcept
	{
		if(IsMapped())
			return vk::Result::eSuccess;

		hrs::assert_true_debug(device, "Device isn't created yet!");

		auto [_map_ptr_res, _map_ptr] = device.mapMemory(memory, 0, size);
		if(_map_ptr_res == vk::Result::eSuccess)
			map_ptr = reinterpret_cast<std::byte *>(_map_ptr);

		return _map_ptr_res;
	}
};
