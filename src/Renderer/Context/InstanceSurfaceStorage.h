#pragma once

#include <vector>
#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
	struct InstanceSurfaceStorageLoader
	{
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;

		InstanceSurfaceStorageLoader() noexcept;

		void Init(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept;
	};

	class InstanceSurfaceStorage
	{
	public:
		InstanceSurfaceStorage() = default;
		~InstanceSurfaceStorage() = default;
		InstanceSurfaceStorage(const InstanceSurfaceStorage &) = default;
		InstanceSurfaceStorage & operator=(const InstanceSurfaceStorage &) = default;
		InstanceSurfaceStorage(InstanceSurfaceStorage &&) = default;
		InstanceSurfaceStorage & operator=(InstanceSurfaceStorage &&) = default;

		void AddSurface(VkSurfaceKHR surface);
		void DeleteSurface(VkInstance instance,
						   VkSurfaceKHR surface,
						   const VkAllocationCallbacks *allocation_callbacks) noexcept;
		bool HasSurface(VkSurfaceKHR surface) const noexcept;

		void Destroy(VkInstance instance, const VkAllocationCallbacks *allocation_callbacks) noexcept;

		const InstanceSurfaceStorageLoader & GetInstanceSurfaceStorageLoader() const noexcept;

	protected:
		InstanceSurfaceStorageLoader instance_surface_storage_loader;
		std::vector<VkSurfaceKHR> surfaces;
	};
};
