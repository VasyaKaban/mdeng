#include "Context.h"
#include "VulkanLibrary.h"

namespace FireLand
{
	Context::Context() noexcept
		: parent_library(nullptr),
		  handle(VK_NULL_HANDLE) {}

	Context::~Context()
	{
		Destroy();
	}

	Context::Context(Context &&ctx) noexcept
		: parent_library(ctx.parent_library),
		  handle(std::exchange(ctx.handle, VK_NULL_HANDLE)),
		  physical_devices(std::move(ctx.physical_devices)),
		  surfaces(std::move(ctx.surfaces)),
		  devices(std::move(ctx.devices)),
		  loader(ctx.loader),
		  allocation_cbacks(ctx.allocation_cbacks) {}

	Context & Context::operator=(Context &&ctx) noexcept
	{
		Destroy();

		parent_library = ctx.parent_library;
		handle = std::exchange(ctx.handle, VK_NULL_HANDLE);
		physical_devices = std::move(ctx.physical_devices);
		surfaces = std::move(ctx.surfaces);
		devices = std::move(ctx.devices);
		loader = ctx.loader;
		allocation_cbacks = ctx.allocation_cbacks;

		return *this;
	}

	VkResult Context::Init(VulkanLibrary *_parent_library,
						   VkInstance _handle,
						   std::shared_ptr<VkAllocationCallbacks> _allocation_cbacks)
	{
		if(!_parent_library || _handle == VK_NULL_HANDLE)
			return VK_ERROR_INITIALIZATION_FAILED;

		Destroy();

		bool loader_res = loader.Init(_handle, _parent_library->GetResolver());
		if(!loader_res)
			return VK_ERROR_INITIALIZATION_FAILED;

		std::vector<VkPhysicalDevice> _physcial_devices;
		if(loader.EnumeratePhysicalDevices)
		{
			std::uint32_t physical_device_count = 0;
			VkResult res = loader.EnumeratePhysicalDevices(_handle, &physical_device_count, nullptr);
			if(res != VK_SUCCESS)
				return res;

			_physcial_devices.resize(physical_device_count);
			res = loader.EnumeratePhysicalDevices(_handle, &physical_device_count, _physcial_devices.data());
			if(res != VK_SUCCESS)
				return res;
		}

		parent_library = _parent_library;
		handle = _handle;
		physical_devices = std::move(_physcial_devices);
		allocation_cbacks = _allocation_cbacks;
		return VK_SUCCESS;
	}

	void Context::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		devices = decltype(devices){};
		if(loader.DestroySurfaceKHR)//MAY LEAK OTHERWISE
			for(auto surf : surfaces)
				loader.DestroySurfaceKHR(handle, surf, allocation_cbacks.get());

		surfaces = decltype(surfaces){};
		physical_devices = decltype(physical_devices){};


		if(loader.DestroyInstance)//MAY LEAK OTHERWISE
			loader.DestroyInstance(handle, allocation_cbacks.get());

		allocation_cbacks.reset();
	}

	hrs::expected<Device *, VkResult>
	Context::CreateDevice(const VkDeviceCreateInfo &info,
						  VkPhysicalDevice physical_device,
						  std::shared_ptr<VkAllocationCallbacks> _allocation_callbacks)
	{
		if(!IsCreated())
			return VK_ERROR_INITIALIZATION_FAILED;

		if(std::ranges::find(physical_devices, physical_device) == physical_devices.end())
			return VK_ERROR_INITIALIZATION_FAILED;

		VkDevice _device;
		VkResult res = loader.CreateDevice(physical_device, &info, allocation_cbacks.get(), &_device);
		if(res != VK_SUCCESS)
			return res;

		Device device;
		res = device.Init(this, _device, physical_device, _allocation_callbacks);
		if(res != VK_SUCCESS)
		{
			loader.DestroyDevice(_device, allocation_cbacks.get());
			return res;
		}

		devices.push_back(std::move(device));
		return &devices.back();
	}

	void Context::DestroyDevice(const Device &device) noexcept
	{
		if(auto it = std::ranges::find(devices, device); it != devices.end())
			devices.erase(it);
	}

	bool Context::IsCreated() const noexcept
	{
		return handle != VK_NULL_HANDLE;
	}

	Context::operator bool() const noexcept
	{
		return IsCreated();
	}

	bool Context::operator==(const Context &ctx) const noexcept
	{
		return parent_library == ctx.parent_library && handle == ctx.handle;
	}

	PFN_vkVoidFunction Context::GetProcAddressRaw(const char *name) const noexcept
	{
		if(!IsCreated())
			return nullptr;

		return loader.GetInstanceProcAddr(handle, name);
	}

	const InstanceLoader & Context::GetLoader() const noexcept
	{
		return loader;
	}

	VulkanLibrary * Context::GetLibrary() noexcept
	{
		return parent_library;
	}

	const VulkanLibrary * Context::GetLibrary() const noexcept
	{
		return parent_library;
	}

	VkInstance Context::GetHandle() const noexcept
	{
		return handle;
	}

	const std::vector<VkPhysicalDevice> & Context::GetPhysicalDevices() const noexcept
	{
		return physical_devices;
	}

	const std::vector<VkSurfaceKHR> & Context::GetSurfaces() const noexcept
	{
		return surfaces;
	}

	std::vector<Device> & Context::GetDevices() noexcept
	{
		return devices;
	}

	const std::vector<Device> & Context::GetDevices() const noexcept
	{
		return devices;
	}

	std::shared_ptr<VkAllocationCallbacks> Context::GetAllocatorCallbacks() const noexcept
	{
		return allocation_cbacks;
	}
};
