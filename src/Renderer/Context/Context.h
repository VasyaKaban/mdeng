#pragma once

#include <vector>
#include <memory>
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "InstanceLoader.h"
#include "Device.h"

namespace FireLand
{
	class VulkanLibrary;

	class Context : public hrs::non_copyable
	{
	public:
		Context() noexcept;
		~Context();
		Context(Context &&ctx) noexcept;
		Context & operator=(Context &&ctx) noexcept;

		VkResult Init(VulkanLibrary *_parent_library,
					  VkInstance _handle,
					  std::shared_ptr<VkAllocationCallbacks> _allocation_cbacks);

		void Destroy() noexcept;

		hrs::expected<Device *, VkResult>
		CreateDevice(const VkDeviceCreateInfo &info,
					 VkPhysicalDevice physical_device,
					 std::shared_ptr<VkAllocationCallbacks> _allocation_callbacks);

		void DestroyDevice(const Device &device) noexcept;

		bool IsCreated() const noexcept;
		explicit operator bool() const noexcept;

		bool operator==(const Context &ctx) const noexcept;

		PFN_vkVoidFunction GetProcAddressRaw(const char *name) const noexcept;

		const InstanceLoader & GetLoader() const noexcept;

		VulkanLibrary * GetLibrary() noexcept;
		const VulkanLibrary * GetLibrary() const noexcept;
		VkInstance GetHandle() const noexcept;
		const std::vector<VkPhysicalDevice> & GetPhysicalDevices() const noexcept;
		const std::vector<VkSurfaceKHR> & GetSurfaces() const noexcept;
		std::vector<Device> & GetDevices() noexcept;
		const std::vector<Device> & GetDevices() const noexcept;
		std::shared_ptr<VkAllocationCallbacks> GetAllocatorCallbacks() const noexcept;

		template<typename P>
			requires std::is_pointer_v<P> && std::is_function_v<std::remove_pointer_t<P>>
		P GetProcAddress(const char * const name) const noexcept
		{
			return reinterpret_cast<P>(GetProcAddressRaw(name));
		}

	private:
		VulkanLibrary *parent_library;
		VkInstance handle;
		std::vector<VkPhysicalDevice> physical_devices;
		std::vector<VkSurfaceKHR> surfaces;
		std::vector<Device> devices;
		InstanceLoader loader;
		std::shared_ptr<VkAllocationCallbacks> allocation_cbacks;
	};
};
