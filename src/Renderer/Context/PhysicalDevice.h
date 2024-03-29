#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include <list>
#include "../../hrs/non_creatable.hpp"
#include "Device.h"

namespace FireLand
{
	class Context;

	class PhysicalDevice : public hrs::non_copyable
	{
	public:
		using DevicesContainer = std::list<Device>;

		PhysicalDevice(const Context *_parent_context, vk::PhysicalDevice _physical_device) noexcept;
		~PhysicalDevice();
		PhysicalDevice(PhysicalDevice &&ph_dev) noexcept;
		PhysicalDevice & operator=(PhysicalDevice &&ph_dev) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;
		vk::PhysicalDevice GetHandle() const noexcept;
		const Context * GetContext() const noexcept;
		hrs::expected<std::reference_wrapper<Device>, vk::Result> CreateDevice(const vk::DeviceCreateInfo &info);

		const DevicesContainer & GetDevices() const noexcept;
		Device & GetDevice(std::size_t index) noexcept;
		const Device & GetDevice(std::size_t index) const noexcept;

		void DropDevice(const Device &device);
		void DropDevice(DevicesContainer::const_iterator it);

	private:
		const Context *parent_context = {};
		vk::PhysicalDevice physical_device;
		DevicesContainer devices;
	};
};
