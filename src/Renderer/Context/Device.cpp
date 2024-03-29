#include "Device.h"
#include "PhysicalDevice.h"
#include "../../hrs/iterator_for_each.hpp"

namespace FireLand
{
	Device::Device(vk::Device _device,
				   const PhysicalDevice *_parent_physical_device,
				   const vk::PhysicalDeviceFeatures &_enabled_features,
				   std::unique_ptr<Allocator> _allocator) noexcept
		: device(std::move(_device)),
		  parent_physical_device(_parent_physical_device),
		  enabled_features(_enabled_features),
		  allocator(std::move(_allocator)) {}

	hrs::expected<Device, vk::Result>
	Device::Create(const PhysicalDevice *_parent_physical_device, const vk::DeviceCreateInfo &info) noexcept
	{
		hrs::assert_true_debug(_parent_physical_device, "Physical device pointer points to null!");
		hrs::assert_true_debug(_parent_physical_device->GetHandle(), "Physical device isn't created yet!");

		auto [_device_res, _device] = _parent_physical_device->GetHandle().createDevice(info);
		if(_device_res != vk::Result::eSuccess)
			return _device_res;

		std::unique_ptr<Allocator> allocator(new Allocator(_device,
														   _parent_physical_device->GetHandle()));

		return Device(_device, _parent_physical_device, *info.pEnabledFeatures, std::move(allocator));
	}

	Device::~Device()
	{
		Destroy();
	}

	Device::Device(Device &&dev) noexcept
		: device(std::exchange(dev.device, VK_NULL_HANDLE)),
		  parent_physical_device(dev.parent_physical_device),
		  enabled_features(dev.enabled_features),
		  workers(std::move(dev.workers)),
		  allocator(std::move(dev.allocator)) {}

	Device & Device::operator=(Device &&dev) noexcept
	{
		Destroy();
		device = dev.device;
		device = std::exchange(dev.device, VK_NULL_HANDLE);
		enabled_features = dev.enabled_features;
		workers = std::move(dev.workers);
		allocator = std::move(dev.allocator);

		dev.device = VK_NULL_HANDLE;
		return *this;
	}

	void Device::Destroy() noexcept
	{
		if(IsCreated())
		{
			[[maybe_unused]]auto _ = device.waitIdle();
			workers.clear();
			allocator.reset();
			device.destroy();
			device = VK_NULL_HANDLE;
		}
	}

	bool Device::IsCreated() const noexcept
	{
		return device;
	}

	vk::Device Device::GetHandle() const noexcept
	{
		return device;
	}

	const PhysicalDevice * Device::GetPhysicalDevice() const noexcept
	{
		return parent_physical_device;
	}

	const Device::DeviceWorkersContainer & Device::GetDeviceWorkers() const noexcept
	{
		return workers;
	}

	DeviceWorker * Device::GetDeviceWorker(std::size_t index) noexcept
	{
		return std::next(workers.begin(), index)->get();
	}

	const DeviceWorker * Device::GetDeviceWorker(std::size_t index) const noexcept
	{
		return std::next(workers.begin(), index)->get();
	}

	void Device::DropDeviceWorker(const DeviceWorker *worker)
	{
		hrs::iterator_for_each(workers, [&](std::list<std::unique_ptr<DeviceWorker>>::iterator iter) -> bool
		{
			if(iter->get() == worker)
			{
				workers.erase(iter);
				return true;
			}

			return false;
		});
	}

	void Device::DropDeviceWorker(DeviceWorkersContainer::const_iterator it)
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(workers, it),
							   "Device worker is not a part of this device!");
	}

	Allocator * Device::GetAllocator() noexcept
	{
		return allocator.get();
	}

	const Allocator * Device::GetAllocator() const noexcept
	{
		return allocator.get();
	}

	const vk::PhysicalDeviceFeatures & Device::GetEnabledFeatures() const noexcept
	{
		return enabled_features;
	}
}
