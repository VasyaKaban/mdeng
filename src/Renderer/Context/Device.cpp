#include "Device.h"
#include "Context.h"

namespace FireLand
{
	Device::Device() noexcept
		: parent_context(nullptr),
		  handle(VK_NULL_HANDLE),
		  physical_device(VK_NULL_HANDLE) {}

	Device::~Device()
	{
		Destroy();
	}

	Device::Device(Device &&dev) noexcept
		: parent_context(dev.parent_context),
		  handle(std::exchange(dev.handle, VK_NULL_HANDLE)),
		  physical_device(std::exchange(dev.physical_device, VK_NULL_HANDLE)),
		  allocation_cbacks(dev.allocation_cbacks),
		  loader(dev.loader),
		  workers(std::move(dev.workers)) {}

	Device & Device::operator=(Device &&dev) noexcept
	{
		Destroy();

		parent_context = dev.parent_context;
		handle = std::exchange(dev.handle, VK_NULL_HANDLE);
		physical_device = std::exchange(dev.physical_device, VK_NULL_HANDLE);
		allocation_cbacks = dev.allocation_cbacks;
		loader = dev.loader;
		workers = std::move(dev.workers);

		return *this;
	}

	VkResult Device::Init(Context *_parent_context,
						  VkDevice _handle,
						  VkPhysicalDevice _physical_device,
						  std::shared_ptr<VkAllocationCallbacks> _allocation_cbacks) noexcept
	{
		Destroy();

		if(!_parent_context || _handle == VK_NULL_HANDLE || _physical_device == VK_NULL_HANDLE)
			return VK_ERROR_INITIALIZATION_FAILED;

		bool loader_res = loader.Init(_handle, _parent_context->GetLoader().GetDeviceProcAddr);
		if(!loader_res)
			return VK_ERROR_INITIALIZATION_FAILED;

		parent_context = _parent_context;
		handle = _handle;
		physical_device = _physical_device;
		allocation_cbacks = _allocation_cbacks;
		return VK_SUCCESS;
	}

	void Device::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		workers = decltype(workers){};
		parent_context->GetLoader().DestroyDevice(handle, parent_context->GetAllocatorCallbacks().get());
		handle = VK_NULL_HANDLE;
		physical_device = VK_NULL_HANDLE;
		allocation_cbacks = {};
	}

	void Device::AddDeviceWorker(DeviceWorker *worker)
	{
		if(auto it = find_worker(worker); it == workers.end())
			workers.push_back(std::unique_ptr<DeviceWorker>(worker));
	}

	void Device::DropDeviceWorker(DeviceWorker *worker) noexcept
	{
		if(auto it = find_worker(worker); it != workers.end())
			workers.erase(it);
	}

	DeviceWorker * Device::ReleaseDeviceWorker(DeviceWorker *worker) noexcept
	{
		if(auto it = find_worker(worker); it != workers.end())
		{
			DeviceWorker *w = it->release();
			workers.erase(it);
			return w;
		}

		return nullptr;
	}

	bool Device::IsCreated() const noexcept
	{
		return handle != VK_NULL_HANDLE;
	}

	Device::operator bool() const noexcept
	{
		return IsCreated();
	}

	bool Device::operator==(const Device &device) const noexcept
	{
		return parent_context == device.parent_context && handle == device.handle;
	}

	PFN_vkVoidFunction Device::GetProcAddressRaw(const char *name) const noexcept
	{
		if(!IsCreated())
			return nullptr;

		return loader.GetDeviceProcAddr(handle, name);
	}

	const DeviceLoader & Device::GetLoader() const noexcept
	{
		return loader;
	}

	Context * Device::GetParentContext() noexcept
	{
		return parent_context;
	}

	const Context * Device::GetParentContext() const noexcept
	{
		return parent_context;
	}

	VkDevice Device::GetHandle() const noexcept
	{
		return handle;
	}

	VkPhysicalDevice Device::GetPhysicalDevice() const noexcept
	{
		return physical_device;
	}

	std::vector<std::unique_ptr<DeviceWorker>> & Device::GetDeviceWorkers() noexcept
	{
		return workers;
	}

	const std::vector<std::unique_ptr<DeviceWorker>> & Device::GetDeviceWorkers() const noexcept
	{
		return workers;
	}

	std::vector<std::unique_ptr<DeviceWorker>>::iterator Device::find_worker(DeviceWorker *worker) noexcept
	{
		return std::ranges::find_if(workers, [worker](const std::unique_ptr<DeviceWorker> &w)
		{
			return worker == w.get();
		});
	}
};
