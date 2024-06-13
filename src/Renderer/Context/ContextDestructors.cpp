#include "ContextDestructors.h"
#include "../Vulkan/VulkanLibrary.h"

namespace FireLand
{
	ContextDestructors::ContextDestructors() noexcept
		: vkDestroyInstance(nullptr),
		  vkDestroyDevice(nullptr) {}

	void ContextDestructors::Init(PFN_vkGetInstanceProcAddr global_vkGetInstanceProcAdd) noexcept
	{
		vkDestroyInstance =
			reinterpret_cast<PFN_vkDestroyInstance>(global_vkGetInstanceProcAdd(VK_NULL_HANDLE,
																				"vkDestroyInstance"));

		vkDestroyDevice =
			reinterpret_cast<PFN_vkDestroyDevice>(global_vkGetInstanceProcAdd(VK_NULL_HANDLE,
																			  "vkDestroyDevice"));
	}
};
