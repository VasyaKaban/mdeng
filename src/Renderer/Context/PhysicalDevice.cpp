#include "PhysicalDevice.h"
#include "Context.h"
#include "../../hrs/debug.hpp"
#include "../../hrs/iterator_for_each.hpp"

namespace FireLand
{
	PhysicalDevice::PhysicalDevice(const Context *_parent_context, vk::PhysicalDevice _physical_device) noexcept
		: parent_context(_parent_context),
		  physical_device(_physical_device) {}

	PhysicalDevice::~PhysicalDevice()
	{
		Destroy();
	}

	PhysicalDevice::PhysicalDevice(PhysicalDevice &&ph_dev) noexcept
		: parent_context(ph_dev.parent_context),
		  physical_device(std::exchange(ph_dev.physical_device, VK_NULL_HANDLE)),
		  devices(std::move(ph_dev.devices)) {}

	PhysicalDevice & PhysicalDevice::operator=(PhysicalDevice &&ph_dev) noexcept
	{
		Destroy();
		parent_context  = ph_dev.parent_context;
		physical_device = std::exchange(ph_dev.physical_device, VK_NULL_HANDLE);
		devices = std::move(ph_dev.devices);
		return *this;
	}

	void PhysicalDevice::Destroy()
	{
		if(IsCreated())
		{
			devices.clear();
			physical_device = VK_NULL_HANDLE;
		}
	}

	bool PhysicalDevice::IsCreated() const noexcept
	{
		return physical_device;
	}

	vk::PhysicalDevice PhysicalDevice::GetHandle() const noexcept
	{
		return physical_device;
	}

	const Context * PhysicalDevice::GetContext() const noexcept
	{
		return parent_context;
	}

	hrs::expected<std::reference_wrapper<Device>, vk::Result>
	PhysicalDevice::CreateDevice(const vk::DeviceCreateInfo &info)
	{
		hrs::assert_true_debug(IsCreated(), "Physical devcie isn't created yet!");

		auto device_exp = Device::Create(this, info);
		if(!device_exp)
			return device_exp.error();

		devices.push_back(std::move(device_exp.value()));
		return std::ref(*std::prev(devices.end()));
	}

	const PhysicalDevice::DevicesContainer & PhysicalDevice::GetDevices() const noexcept
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
			if(iter->GetHandle() == device.GetHandle())
			{
				devices.erase(iter);
				return true;
			}

			return false;
		});
	}

	void PhysicalDevice::DropDevice(DevicesContainer::const_iterator it)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(devices, it),
							   "Devices is not a part of this physical device!");

		devices.erase(it);
	}
};
