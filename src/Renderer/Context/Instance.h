#pragma once

#include <vector>
#include <memory>
#include <optional>
#include "hrs/non_creatable.hpp"
#include "InstanceLoader.h"

namespace FireLand
{
	class Context;
	class Device;

	class Instance : public hrs::non_copyable
	{
	public:
		Instance() noexcept;
		virtual ~Instance();
		Instance(Instance &&i) noexcept;
		Instance & operator=(Instance &&i) noexcept;

		VkResult Init(Context *_parent_context,
						const VkInstanceCreateInfo &info,
						const VkAllocationCallbacks *_allocation_callbacks) noexcept;

		virtual void Destroy() noexcept;

		virtual bool IsCreated() const noexcept;
		virtual explicit operator bool() const noexcept;

		void AddDevice(Device *dev);
		void DeleteDevice(Device *dev) noexcept;
		bool HasDevice(Device *dev) const noexcept;

		Context * GetParentContext() noexcept;
		const Context * GetParentContext() const noexcept;
		VkInstance GetHandle() const noexcept;
		const InstanceLoader & GetInstanceLoader() const noexcept;
		const VkAllocationCallbacks * GetAllocationCallbacks() const noexcept;

	protected:
		Context *parent_context;
		VkInstance handle;
		InstanceLoader instance_loader;
		std::optional<VkAllocationCallbacks> allocation_callbacks;
		std::vector<std::unique_ptr<Device>> devices;
	};
};
