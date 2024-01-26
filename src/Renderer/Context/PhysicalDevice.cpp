#include "PhysicalDevice.h"
#include "../../hrs/debug.hpp"
#include "../../hrs/iterator_for_each.hpp"

namespace FireLand
{
	PhysicalDevice::PhysicalDevice(vk::PhysicalDevice _physical_device)
		: physical_device(_physical_device) {}

	PhysicalDevice::~PhysicalDevice()
	{
		Destroy();
	}

	PhysicalDevice::PhysicalDevice(PhysicalDevice &&ph_dev) noexcept
		: physical_device(ph_dev.physical_device), devices(std::move(ph_dev.devices))
	{
		ph_dev.physical_device = VK_NULL_HANDLE;
	}

	PhysicalDevice & PhysicalDevice::operator=(PhysicalDevice &&ph_dev) noexcept
	{
		Destroy();
		physical_device = ph_dev.physical_device;
		devices = std::move(ph_dev.devices);
		ph_dev.physical_device = VK_NULL_HANDLE;
		return *this;
	}

	void PhysicalDevice::Destroy()
	{
		if(IsCreated())
		{
			for(auto &dev : devices)
				dev.Destroy();

			physical_device = VK_NULL_HANDLE;
		}
	}

	bool PhysicalDevice::IsCreated() const noexcept
	{
		return physical_device;
	}

	vk::PhysicalDevice PhysicalDevice::GetPhysicalDeviceHandle() const noexcept
	{
		return physical_device;
	}

	hrs::expected<std::reference_wrapper<Device>, vk::Result>
	PhysicalDevice::CreateDevice(const vk::DeviceCreateInfo &info)
	{
		hrs::assert_true_debug(IsCreated(), "Physical devcie isn't created yet!");

		auto device_exp = Device::Create(physical_device, info);
		if(!device_exp)
			return device_exp.error();

		devices.push_back(std::move(device_exp.value()));
		return std::ref(*std::prev(devices.end()));
	}

	const std::list<Device> & PhysicalDevice::GetDevices() const noexcept
	{
		return devices;
	}

	Device & PhysicalDevice::GetDevice(std::size_t index) noexcept
	{
		return *std::next(devices.begin(), index);
	}

	const Device & PhysicalDevice::GetDevice(std::size_t index) const noexcept
	{
		return *std::next(devices.begin(), index);
	}

	void PhysicalDevice::DropDevice(const Device &device)
	{
		hrs::iterator_for_each(devices, [&](std::list<Device>::iterator iter) -> bool
		{
			if(iter->GetDevice() == device.GetDevice())
			{
				iter->Destroy();
				devices.erase(iter);
				return true;
			}

			return false;
		});
	}
};
