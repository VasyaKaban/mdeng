#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/expected.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/instantiation.hpp"
#include "../../Allocator/Allocator.hpp"
#include "DeviceWorker.h"
#include <memory>
#include <list>

namespace FireLand
{
	class Device : public hrs::non_copyable
	{
		Device(vk::Device &&_device,
			   vk::PhysicalDevice &_physical_device,
			   const vk::PhysicalDeviceFeatures &_features,
			   std::unique_ptr<Allocator> _allocator);
	public:
		static hrs::expected<Device, vk::Result>
		Create(vk::PhysicalDevice physical_device, const vk::DeviceCreateInfo &info) noexcept;

		~Device();
		Device(Device &&dev) noexcept;
		Device & operator=(Device &&dev) noexcept;

		void Destroy() noexcept;

		bool IsCreated() const noexcept;

		vk::Device GetDevice() const noexcept;
		vk::PhysicalDevice GetPhysicalDevice() const noexcept;

		const std::list<std::unique_ptr<DeviceWorker>> & GetWorkers() const noexcept;
		DeviceWorker * GetWorker(std::size_t index) noexcept;
		const DeviceWorker * GetWorker(std::size_t index) const noexcept;

		void DropDeviceWorker(const DeviceWorker *worker) noexcept;

		Allocator * GetAllocator() noexcept;
		const Allocator * GetAllocator() const noexcept;

		const vk::PhysicalDeviceFeatures & GetFeatures() const noexcept;

		template<typename W, typename ...Args>
			requires
				DeviceWorkerConcept<W, Device *, Args...>
		auto CreateDeviceWorker(Args &&...args);

	private:
		vk::Device device;
		vk::PhysicalDevice physical_device;
		vk::PhysicalDeviceFeatures features;
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
