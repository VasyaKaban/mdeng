#include <dlfcn.h>
#include <cstring>
#include <cassert>
#include "VulkanLibrary.h"

namespace FireLand
{
	VulkanLibrary::VulkanLibrary() noexcept
		: lib({}),
		  vkGetInstanceProcAddr(nullptr) {}

	VulkanLibrary::~VulkanLibrary()
	{
		Close();
	}

	VulkanLibrary::VulkanLibrary(VulkanLibrary &&vl) noexcept
		: lib(std::move(vl.lib)),
		  vkGetInstanceProcAddr(std::exchange(vl.vkGetInstanceProcAddr, nullptr)) {}

	VulkanLibrary & VulkanLibrary::operator=(VulkanLibrary &&vl) noexcept
	{
		Close();

		lib = std::move(vl.lib);
		vkGetInstanceProcAddr = std::exchange(vl.vkGetInstanceProcAddr, nullptr);

		return *this;
	}

	void VulkanLibrary::Close() noexcept
	{
		lib.close();
		vkGetInstanceProcAddr = nullptr;
	}

	std::optional<std::size_t> VulkanLibrary::Open(std::span<const char * const> names) noexcept
	{
		if(names.empty())
			return {};

		if(IsOpen())
			Close();

		for(std::size_t i = 0; i < names.size(); i++)
		{
			if(names[i])
			{
				bool open_res = lib.open(names[i]);
				if(open_res)
				{
					vkGetInstanceProcAddr = lib.get_ptr<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
					if(vkGetInstanceProcAddr)
						return i;
					else
						lib.close();
				}
			}
		}

		return {};
	}

	bool VulkanLibrary::IsOpen() const noexcept
	{
		return lib.is_open();
	}

	VulkanLibrary::operator bool() const noexcept
	{
		return IsOpen();
	}

	PFN_vkGetInstanceProcAddr VulkanLibrary::GetResolver() const noexcept
	{
		return vkGetInstanceProcAddr;
	}

	PFN_vkVoidFunction VulkanLibrary::GetProcAddressRaw(const char *name) const noexcept
	{
		if(vkGetInstanceProcAddr)
			return vkGetInstanceProcAddr(VK_NULL_HANDLE, name);

		return nullptr;
	}
};

