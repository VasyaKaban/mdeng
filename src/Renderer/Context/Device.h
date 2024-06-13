#pragma once

#include <vector>
#include <memory>
#include <optional>
#include "hrs/non_creatable.hpp"
#include "DeviceLoader.h"

namespace FireLand
{
	class Instance;
	class DeviceWorker;

	class Device : public hrs::non_copyable
	{
	public:
		Device() noexcept;
		virtual ~Device();
		Device(Device &&dev) noexcept;
		Device & operator=(Device &&dev) noexcept;

		VkResult Init(Instance *_parent_instance,
					  VkPhysicalDevice physical_device,
					  const VkDeviceCreateInfo &info,
					  const VkAllocationCallbacks *_allocation_callbacks) noexcept;

		virtual void Destroy() noexcept;

		virtual bool IsCreated() const noexcept;
		virtual explicit operator bool() const noexcept;

		void AddDeviceWorker(DeviceWorker *dev_worker);
		void DeleteDeviceWorker(DeviceWorker *dev_worker) noexcept;
		bool HasDeviceWorker(DeviceWorker *dev_worker) const noexcept;

		Instance * GetParentInstance() noexcept;
		const Instance * GetParentInstance() const noexcept;
		VkDevice GetHandle() const noexcept;
		const DeviceLoader & GetDeviceLoader() const noexcept;
		const VkAllocationCallbacks * GetAllocationCallbacks() const noexcept;

	protected:
		Instance *parent_instance;
		VkDevice handle;
		DeviceLoader device_loader;
		std::optional<VkAllocationCallbacks> allocation_callbacks;
		std::vector<std::unique_ptr<DeviceWorker>> device_workers;
	};
};
