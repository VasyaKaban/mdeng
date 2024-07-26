#pragma once

#include <array>
#include <vector>
#include <string>
#include <span>
#include "hrs/dynamic_library.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/debug.hpp"
#include "GlobalLoader.h"
#include "InstanceLoader.h"
#include "DeviceUtilizer.h"

namespace FireLand
{
	class Context : public hrs::non_copyable
	{
	public:
#if defined(unix) || defined(__unix) || defined(__unix__)
		constexpr static std::array DEFAULT_NAMES = {"libvulkan.so", "libvulkan.so.1"};
#elif defined(_WIN32)
		constexpr static std::array DEFAULT_NAMES = {"vulkan-1.dll"};
#elif defined(__APPLE__) || defined(__MACH__) || defined(macintosh) || defined(Macintosh)
		constexpr static std::array DEFAULT_NAMES = {"libvulkan.dylib", "libvulkan.1.dylib"};
#endif

	private:
		Context(hrs::dynamic_library &&_vk_library,
				const char * const _vk_libary_name,
				const GlobalLoader &_global_loader);
	public:

		Context() noexcept;
		~Context();
		Context(Context &&ctx) noexcept;
		Context & operator=(Context &&ctx) noexcept;

		static hrs::expected<Context, VkResult> Init(std::span<const char * const> library_names = DEFAULT_NAMES);
		VkResult Create(const VkInstanceCreateInfo &info,
						const VkAllocationCallbacks *_allocation_callbacks);

		void Destroy() noexcept;

		void AddSurface(VkSurfaceKHR surface);
		void DestroySurface(VkSurfaceKHR surface) noexcept;

		hrs::expected<VkDebugUtilsMessengerEXT, VkResult>
		CreateDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT &info);

		void DestroyDebugMessenger(VkDebugUtilsMessengerEXT messenger) noexcept;

		void DestroyDeviceUtilizer(DeviceUtilizer *du) noexcept;

		const GlobalLoader & GetGlobalLoader() const noexcept;
		const InstanceLoader & GetInstanceLoader() const noexcept;

		const std::string & GetVulkanLibraryName() const noexcept;
		VkInstance GetHandle() const noexcept;
		const std::vector<VkPhysicalDevice> & GetPhysicalDevices() const noexcept;
		const std::vector<VkSurfaceKHR> & GetSurfaces() const noexcept;
		const std::vector<VkDebugUtilsMessengerEXT> & GetDebugMessengers() const noexcept;
		std::vector<std::unique_ptr<DeviceUtilizer>> & GetDeviceUtilizers() noexcept;
		const std::vector<std::unique_ptr<DeviceUtilizer>> & GetDeviceUtilizers() const noexcept;
		const VkAllocationCallbacks * GetAllocationCallbacks() const noexcept;

		bool IsInitialized() const noexcept;
		bool IsCreated() const noexcept;

		template<typename DU, typename ...Args>
		hrs::expected<DU *, hrs::error> CreateDeviceUtilizer(VkPhysicalDevice physical_device,
															 const VkDeviceCreateInfo &info,
															 Args &&...args)
			requires DeviceUtilizerCreatable<DU, VkDevice, Args...>;

	private:
		hrs::dynamic_library vk_library;
		std::string vk_library_name;
		GlobalLoader global_loader;
		VkInstance instance;
		InstanceLoader instance_loader;
		std::vector<VkPhysicalDevice> physical_devices;
		std::vector<VkSurfaceKHR> surfaces;
		std::vector<VkDebugUtilsMessengerEXT> debug_messengers;
		std::vector<std::unique_ptr<DeviceUtilizer>> device_utilizers;
		std::optional<VkAllocationCallbacks> allocation_callbacks;
	};

	template<typename DU, typename ...Args>
	hrs::expected<DU *, hrs::error> Context::CreateDeviceUtilizer(VkPhysicalDevice physical_device,
																  const VkDeviceCreateInfo &info,
																  Args &&...args)
		requires DeviceUtilizerCreatable<DU, VkDevice, Args...>
	{
		hrs::assert_true_debug(IsCreated(), "Context isn't created yet!");
		hrs::assert_true_debug(std::ranges::find(physical_devices, physical_device) != physical_devices.end(),
							   "Passed physical device: {} is not a part of this context!",
							   static_cast<void *>(physical_device));

		hrs::assert_true(instance_loader.vkCreateDevice != nullptr,
						 "Instance loader didn't load vkCreateDevice!");

		VkDevice device;
		VkResult res = instance_loader.vkCreateDevice(physical_device,
													  &info,
													  GetAllocationCallbacks(),
													  &device);

		if(res != VK_SUCCESS)
			return res;

		auto du_exp = DU::Create(this, device, std::forward<Args>(args)...);
		if(!du_exp)
		{
			if(instance_loader.vkDestroyDevice)
				instance_loader.vkDestroyDevice(device, GetAllocationCallbacks());
			//else
				//MSG: VkDevice LEAK!!!
			return du_exp.error();
		}

		device_utilizers.push_back(std::unique_ptr<DeviceUtilizer>(new DeviceUtilizer(std::move(*du_exp))));
		return &device_utilizers.back();
	}
};
