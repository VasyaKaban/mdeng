#pragma once

#include <memory>
#include <vector>
#include "hrs/non_creatable.hpp"
#include "DeviceLoader.h"
#include "DeviceWorker.h"

namespace FireLand
{
	class Context;

	class Device : public hrs::non_copyable
	{
	public:
		Device() noexcept;
		~Device();
		Device(Device &&dev) noexcept;
		Device & operator=(Device &&dev) noexcept;

		VkResult Init(Context *_parent_context,
					  VkDevice _handle,
					  VkPhysicalDevice _physical_device,
					  std::shared_ptr<VkAllocationCallbacks> _allocation_cbacks) noexcept;

		void Destroy() noexcept;

		void AddDeviceWorker(DeviceWorker *worker);
		void DropDeviceWorker(DeviceWorker *worker) noexcept;
		DeviceWorker * ReleaseDeviceWorker(DeviceWorker *worker) noexcept;

		bool IsCreated() const noexcept;
		explicit operator bool() const noexcept;

		bool operator==(const Device &device) const noexcept;

		PFN_vkVoidFunction GetProcAddressRaw(const char *name) const noexcept;

		const DeviceLoader & GetLoader() const noexcept;
		Context * GetParentContext() noexcept;
		const Context * GetParentContext() const noexcept;
		VkDevice GetHandle() const noexcept;
		VkPhysicalDevice GetPhysicalDevice() const noexcept;
		std::vector<std::unique_ptr<DeviceWorker>> & GetDeviceWorkers() noexcept;
		const std::vector<std::unique_ptr<DeviceWorker>> & GetDeviceWorkers() const noexcept;

		template<typename P>
			requires std::is_pointer_v<P> && std::is_function_v<std::remove_pointer_t<P>>
		P GetProcAddress(const char * const name) const noexcept
		{
			return reinterpret_cast<P>(GetProcAddressRaw(name));
		}

	private:
		std::vector<std::unique_ptr<DeviceWorker>>::iterator find_worker(DeviceWorker *worker) noexcept;
	private:
		Context *parent_context;
		VkDevice handle;
		VkPhysicalDevice physical_device;
		std::shared_ptr<VkAllocationCallbacks> allocation_cbacks;
		DeviceLoader loader;
		std::vector<std::unique_ptr<DeviceWorker>> workers;
	};
};
