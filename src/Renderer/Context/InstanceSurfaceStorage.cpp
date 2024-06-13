#include "InstanceSurfaceStorage.h"
#include "../Vulkan/Generated/InstanceModulesView.h"

namespace FireLand
{
	void InstanceSurfaceStorage::AddSurface(VkSurfaceKHR surface)
	{
		if(surface == VK_NULL_HANDLE)
			return;

		surfaces.push_back(surface);
	}

	void InstanceSurfaceStorage::DeleteSurface(VkInstance instance,
											   VkSurfaceKHR surface,
											   const VkAllocationCallbacks *allocation_callbacks) noexcept
	{
		auto it = std::ranges::find(surfaces, surface);
		if(it != surfaces.end())
		{
			instance_module_vk_khr_surface.vkDestroySurfaceKHR(instance, surface, allocation_callbacks);
			surfaces.erase(it);
		}
	}

	bool InstanceSurfaceStorage::HasSurface(VkSurfaceKHR surface) const noexcept
	{
		auto it = std::ranges::find(surfaces, surface);
		return it != surfaces.end();
	}

	void InstanceSurfaceStorage::Destroy(VkInstance instance,
										 const VkAllocationCallbacks *allocation_callbacks) noexcept
	{
		for(const auto &surface : surfaces)
			instance_module_vk_khr_surface.vkDestroySurfaceKHR(instance, surface, allocation_callbacks);

		surfaces.clear();
	}

	void InstanceSurfaceStorage::FillModulesView(VkInstance instance,
												 PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
												 InstanceModulesView &modules_view) noexcept
	{
		instance_module_vk_khr_surface.Init(instance, vkGetInstanceProcAddr);
		modules_view._InstanceModule_VK_KHR_surface = &instance_module_vk_khr_surface;
	}
};
