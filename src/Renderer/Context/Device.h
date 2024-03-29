#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/expected.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/instantiation.hpp"
#include "../Allocator/Allocator.h"
#include "DeviceWorker.h"
#include <memory>
#include <list>

namespace FireLand
{
	class PhysicalDevice;

	class Device : public hrs::non_copyable
	{
		Device(vk::Device _device,
			   const PhysicalDevice *_parent_physical_device,
			   const vk::PhysicalDeviceFeatures &_enabled_features,
			   std::unique_ptr<Allocator> _allocator) noexcept;
	public:
		using DeviceWorkersContainer = std::list<std::unique_ptr<DeviceWorker>>;

		static hrs::expected<Device, vk::Result>
		Create(const PhysicalDevice *_parent_physical_device, const vk::DeviceCreateInfo &info) noexcept;

		~Device();
		Device(Device &&dev) noexcept;
		Device & operator=(Device &&dev) noexcept;

		void Destroy() noexcept;

		bool IsCreated() const noexcept;

		vk::Device GetHandle() const noexcept;
		const PhysicalDevice * GetPhysicalDevice() const noexcept;

		const DeviceWorkersContainer & GetDeviceWorkers() const noexcept;
		DeviceWorker * GetDeviceWorker(std::size_t index) noexcept;
		const DeviceWorker * GetDeviceWorker(std::size_t index) const noexcept;

		void DropDeviceWorker(const DeviceWorker *worker);
		void DropDeviceWorker(DeviceWorkersContainer::const_iterator it);

		Allocator * GetAllocator() noexcept;
		const Allocator * GetAllocator() const noexcept;

		const vk::PhysicalDeviceFeatures & GetEnabledFeatures() const noexcept;

		template<typename W, typename ...Args>
			requires
				DeviceWorkerConcept<W, Device *, Args...>
		auto CreateDeviceWorker(Args &&...args);

	private:
		vk::Device device;
		const PhysicalDevice *parent_physical_device;
		vk::PhysicalDeviceFeatures enabled_features;
		std::list<std::unique_ptr<DeviceWorker>> workers;
		std::unique_ptr<Allocator> allocator;
	};

	template<typename W, typename ...Args>
		requires
			DeviceWorkerConcept<W, Device *, Args...>
	auto Device::CreateDeviceWorker(Args &&...args)
	{
		using return_type = hrs::expected<W *,
										  typename decltype(W::Create(this, std::forward<Args>(args)...))::error_type>;

		auto create_res = W::Create(this, std::forward<Args>(args)...);
		if(!create_res)
			return return_type(std::move(create_res.error()));

		std::unique_ptr<W> u_worker(new W(std::move(create_res.value())));
		W *worker_ptr = u_worker.get();
		workers.push_back(std::move(u_worker));
		return return_type(worker_ptr);
	}
};
