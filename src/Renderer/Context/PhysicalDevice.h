#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include <list>
#include "../../hrs/non_creatable.hpp"
#include "Device.h"

namespace FireLand
{
	class PhysicalDevice : public hrs::non_copyable
	{
	public:

		PhysicalDevice(vk::PhysicalDevice _physical_device);
		~PhysicalDevice();
		PhysicalDevice(PhysicalDevice &&ph_dev) noexcept;
		PhysicalDevice & operator=(PhysicalDevice &&ph_dev) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;
		vk::PhysicalDevice GetPhysicalDeviceHandle() const noexcept;
		hrs::expected<std::reference_wrapper<Device>, vk::Result> CreateDevice(const vk::DeviceCreateInfo &info);

		const std::list<Device> & GetDevices() const noexcept;
		Device & GetDevice(std::size_t index) noexcept;
		const Device & GetDevice(std::size_t index) const noexcept;

		void DropDevice(const Device &device);

	private:
		vk::PhysicalDevice physical_device;
		std::list<Device> devices;
	};
};
