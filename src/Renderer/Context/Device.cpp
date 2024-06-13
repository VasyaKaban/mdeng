#include "Device.h"
#include "Instance.h"
#include "Context.h"

namespace FireLand
{
	Device::Device() noexcept
		: parent_instance(nullptr),
		  handle(VK_NULL_HANDLE) {}

	Device::~Device()
	{
		Destroy();
	}

	Device::Device(Device &&dev) noexcept
		: parent_instance(dev.parent_instance),
		  handle(std::exchange(dev.handle, VK_NULL_HANDLE)),
		  device_loader(dev.device_loader),
		  allocation_callbacks(std::move(dev.allocation_callbacks)),
		  device_workers(std::move(dev.device_workers)) {}

	Device & Device::operator=(Device &&dev) noexcept
	{
		Destroy();

		parent_instance = dev.parent_instance;
		handle = std::exchange(dev.handle, VK_NULL_HANDLE);
		device_loader = dev.device_loader;
		allocation_callbacks = std::move(dev.allocation_callbacks);
		device_workers = std::move(dev.device_workers);

		return *this;
	}

	VkResult Device::Init(Instance *_parent_instance,
						  VkPhysicalDevice physical_device,
						  const VkDeviceCreateInfo &info,
						  const VkAllocationCallbacks *_allocation_callbacks) noexcept
	{
		Destroy();

		VkDevice _handle;
		VkResult res = _parent_instance->GetInstanceLoader().vkCreateDevice(physical_device,
																			&info,
																			_allocation_callbacks,
																			&_handle);

		if(res != VkResult::VK_SUCCESS)
			return res;

		DeviceLoader _device_loader;
		bool init_res = _device_loader.Init(_handle,
											_parent_instance->GetInstanceLoader().vkGetDeviceProcAddr);

		if(!init_res)
		{
			const ContextDestructors &context_destructors
				= _parent_instance->GetParentContext()->GetContextDestructors();
			if(context_destructors.vkDestroyDevice)
				context_destructors.vkDestroyDevice(_handle, _allocation_callbacks);

			//Maybe VkResult leak!
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;
		}

		parent_instance = _parent_instance;
		handle = _handle;
		device_loader = _device_loader;
		if(_allocation_callbacks)
			allocation_callbacks = *_allocation_callbacks;
		else
			allocation_callbacks = {};

		return VkResult::VK_SUCCESS;
	}

	void Device::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		device_workers.clear();
		device_loader.vkDestroyDevice(handle, GetAllocationCallbacks());
		handle = VK_NULL_HANDLE;
	}

	bool Device::IsCreated() const noexcept
	{
		return handle != VK_NULL_HANDLE;
	}

	Device::operator bool() const noexcept
	{
		return IsCreated();
	}

	void Device::AddDeviceWorker(DeviceWorker *dev_worker)
	{
		if(!(IsCreated() && dev_worker))
			return;

		device_workers.push_back(std::unique_ptr<DeviceWorker>(dev_worker));
	}

	void Device::DeleteDeviceWorker(DeviceWorker *dev_worker) noexcept
	{
		auto it = std::ranges::find_if(device_workers, [dev_worker](const std::unique_ptr<DeviceWorker> &u_dw)
		{
			return u_dw.get() == dev_worker;
		});

		if(it != device_workers.end())
			device_workers.erase(it);
	}

	bool Device::HasDeviceWorker(DeviceWorker *dev_worker) const noexcept
	{
		auto it = std::ranges::find_if(device_workers, [dev_worker](const std::unique_ptr<DeviceWorker> &u_dw)
		{
			return u_dw.get() == dev_worker;
		});

		return it != device_workers.end();
	}

	Instance * Device::GetParentInstance() noexcept
	{
		return parent_instance;
	}

	const Instance * Device::GetParentInstance() const noexcept
	{
		return parent_instance;
	}

	VkDevice Device::GetHandle() const noexcept
	{
		return handle;
	}

	const DeviceLoader & Device::GetDeviceLoader() const noexcept
	{
		return device_loader;
	}

	const VkAllocationCallbacks * Device::GetAllocationCallbacks() const noexcept
	{
		if(allocation_callbacks)
			return &*allocation_callbacks;

		return nullptr;
	}
};
