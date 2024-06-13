#include "Instance.h"
#include "Context.h"
#include "../Vulkan/VulkanModuleResult.hpp"

namespace FireLand
{
	Instance::Instance() noexcept
		: parent_context(nullptr),
		  handle(VK_NULL_HANDLE) {}

	Instance::~Instance()
	{
		Destroy();
	}

	Instance::Instance(Instance &&i) noexcept
		: parent_context(i.parent_context),
		  handle(std::exchange(i.handle, VK_NULL_HANDLE)),
		  instance_loader(i.instance_loader),
		  allocation_callbacks(std::move(i.allocation_callbacks)),
		  devices(std::move(i.devices)) {}

	Instance & Instance::operator=(Instance &&i) noexcept
	{
		Destroy();

		parent_context = i.parent_context;
		handle = std::exchange(i.handle, VK_NULL_HANDLE);
		instance_loader = i.instance_loader;
		allocation_callbacks = std::move(i.allocation_callbacks);
		devices = std::move(i.devices);

		return *this;
	}

	VkResult Instance::Init(Context *_parent_context,
							const VkInstanceCreateInfo &info,
							const VkAllocationCallbacks *_allocation_callbacks) noexcept
	{
		VkInstance _handle;
		VkResult res = _parent_context->GetGlobalLoader()
						   .vkCreateInstance(&info, _allocation_callbacks, &_handle);
		if(res != VkResult::VK_SUCCESS)
			return res;

		InstanceLoader _instance_loader;
		bool init_res = _instance_loader.Init(_handle,
											  _parent_context->GetGlobalLoader().vkGetInstanceProcAddr);

		if(!init_res)
		{
			const ContextDestructors &context_destructors = _parent_context->GetContextDestructors();
			if(context_destructors.vkDestroyInstance)
				context_destructors.vkDestroyInstance(_handle, _allocation_callbacks);

			//VkInstance leaf is vkDestroyInstance is null!!!
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;
		}

		parent_context = _parent_context;
		handle = _handle;
		instance_loader = _instance_loader;
		if(_allocation_callbacks)
			allocation_callbacks = *_allocation_callbacks;
		else
			allocation_callbacks = {};

		return VkResult::VK_SUCCESS;
	}

	void Instance::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		devices.clear();
		instance_loader.vkDestroyInstance(handle, GetAllocationCallbacks());
		handle = VK_NULL_HANDLE;
	}

	bool Instance::IsCreated() const noexcept
	{
		return handle != VK_NULL_HANDLE;
	}

	Instance::operator bool() const noexcept
	{
		return IsCreated();
	}

	void Instance::AddDevice(Device *dev)
	{
		if(!(IsCreated() && dev))
			return;

		devices.push_back(std::unique_ptr<Device>(dev));
	}

	void Instance::DeleteDevice(Device *dev) noexcept
	{
		auto it = std::ranges::find_if(devices, [dev](const std::unique_ptr<Device> &u_dev)
		{
			return u_dev.get() == dev;
		});

		if(it != devices.end())
			devices.erase(it);
	}

	bool Instance::HasDevice(Device *dev) const noexcept
	{
		auto it = std::ranges::find_if(devices, [dev](const std::unique_ptr<Device> &u_dev)
		{
			return u_dev.get() == dev;
		});

		return it != devices.end();
	}

	Context * Instance::GetParentContext() noexcept
	{
		return parent_context;
	}

	const Context * Instance::GetParentContext() const noexcept
	{
		return parent_context;
	}

	VkInstance Instance::GetHandle() const noexcept
	{
		return handle;
	}

	const InstanceLoader & Instance::GetInstanceLoader() const noexcept
	{
		return instance_loader;
	}

	const VkAllocationCallbacks * Instance::GetAllocationCallbacks() const noexcept
	{
		if(allocation_callbacks)
			return &*allocation_callbacks;

		return nullptr;
	}
};
