#include "Device.h"
#include "../../hrs/iterator_for_each.hpp"

namespace FireLand
{
	Device::Device(vk::Device &&_device,
				   vk::PhysicalDevice &_physical_device,
				   const vk::PhysicalDeviceFeatures &_features,
				   std::unique_ptr<Allocator> _allocator)
		: device(std::move(_device)),
		  physical_device(_physical_device),
		  features(_features),
		  allocator(std::move(_allocator)){}

	hrs::expected<Device, vk::Result>
	Device::Create(vk::PhysicalDevice physical_device, const vk::DeviceCreateInfo &info) noexcept
	{
		hrs::assert_true_debug(physical_device, "Physical device isn't create yet!");

		auto dev_opt = physical_device.createDevice(info);
		if(dev_opt.result != vk::Result::eSuccess)
			return dev_opt.result;

		std::unique_ptr<Allocator> allocator(new Allocator(dev_opt.value, physical_device));
		return Device(std::move(dev_opt.value), physical_device, *info.pEnabledFeatures, std::move(allocator));
	}

	Device::~Device()
	{
		Destroy();
	}

	Device::Device(Device &&dev) noexcept
		: device(dev.device), physical_device(dev.physical_device),
		  features(dev.features), workers(std::move(dev.workers)),
		  allocator(std::move(dev.allocator))
	{
		dev.device = VK_NULL_HANDLE;
		dev.physical_device = VK_NULL_HANDLE;
	}

	Device & Device::operator=(Device &&dev) noexcept
	{
		Destroy();
		device = dev.device;
		physical_device = dev.physical_device;
		features = dev.features;
		workers = std::move(dev.workers);
		allocator = std::move(dev.allocator);

		dev.device = VK_NULL_HANDLE;
		dev.physical_device = VK_NULL_HANDLE;
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
			physical_device = VK_NULL_HANDLE;
		}
	}

	bool Device::IsCreated() const noexcept
	{
		return device;
	}

	vk::Device Device::GetDevice() const noexcept
	{
		return device;
	}

	vk::PhysicalDevice Device::GetPhysicalDevice() const noexcept
	{
		return physical_device;
	}

	const std::list<std::unique_ptr<DeviceWorker>> & Device::GetWorkers() const noexcept
	{
		return workers;
	}

	DeviceWorker * Device::GetWorker(std::size_t index) noexcept
	{
		return std::next(workers.begin(), index)->get();
	}

	const DeviceWorker * Device::GetWorker(std::size_t index) const noexcept
	{
		return std::next(workers.begin(), index)->get();
	}

	void Device::DropDeviceWorker(const DeviceWorker *worker) noexcept
	{
		hrs::iterator_for_each(workers, [&](std::list<std::unique_ptr<DeviceWorker>>::iterator iter) -> bool
		{
			if(iter->get() == worker)
			{
				//iter->reset();
				workers.erase(iter);
				return true;
			}

			return false;
		});
	}

	Allocator * Device::GetAllocator() noexcept
	{
		return allocator.get();
	}

	const Allocator * Device::GetAllocator() const noexcept
	{
		return allocator.get();
	}

	const vk::PhysicalDeviceFeatures & Device::GetFeatures() const noexcept
	{
		return features;
	}
}
