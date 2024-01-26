#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/expected.hpp"
#include "../../hrs/non_creatable.hpp"
#include "../../hrs/instantiation.hpp"
#include "PhysicalDevice.h"
#include "Surface.h"

namespace FireLand
{
	template<typename F>
	concept SurfaceConnector = requires(F &&f)
	{
		{std::forward<F>(f)(vk::Instance{})} -> hrs::instantiation<hrs::expected>;
		requires std::same_as<vk::SurfaceKHR, typename decltype(std::forward<F>(f)(vk::Instance{}))::value_type>;
		requires std::move_constructible<typename decltype(std::forward<F>(f)(vk::Instance{}))::error_type>;
	};

	class Context : public hrs::non_copyable
	{
	private:
		Context(vk::Instance &&_instance, std::vector<PhysicalDevice> &&_physical_devices) noexcept;
	public:

		static hrs::expected<std::uint32_t, vk::Result> GetVersion() noexcept;
		static hrs::expected<std::vector<vk::ExtensionProperties>, vk::Result>
		GetSupportedExtensions(const std::string &layer_name = {});
		static hrs::expected<std::vector<vk::LayerProperties>, vk::Result> GetSupportedLayers();
		static hrs::expected<Context, vk::Result> Create(const vk::InstanceCreateInfo &info);
		~Context();
		Context(Context &&ctx) noexcept;
		Context & operator=(Context &&ctx) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;

		const std::vector<PhysicalDevice> & GetPhysicalDevices() const noexcept;
		PhysicalDevice & GetPhysicalDevice(std::size_t index) noexcept;
		const PhysicalDevice & GetPhysicalDevice(std::size_t index) const noexcept;

		const std::list<Surface> & GetSurfaces() const noexcept;
		Surface & GetSurface(std::size_t index) noexcept;
		const Surface & GetSurface(std::size_t index) const noexcept;

		void DropSurface(const Surface &surface) noexcept;

		vk::Instance GetInstance() const noexcept;

		template<SurfaceConnector F>
		auto CreateSurface(F &&connect_func); //noexcept(std::is_nothrow_invocable_v<F, const vk::Instance &>);

	private:
		vk::Instance instance;
		std::vector<PhysicalDevice> physical_devices;
		std::list<Surface> surfaces;
	};

	template<SurfaceConnector F>
	auto Context::CreateSurface(F &&connect_func) //noexcept(std::is_nothrow_invocable_v<F, const vk::Instance &>)
	{
		using return_type = hrs::expected<std::reference_wrapper<Surface>,
										  typename decltype(std::forward<F>(connect_func)(instance))::error_type>;

		auto result = std::forward<F>(connect_func)(instance);
		if(!result)
			return return_type(std::move(result.error()));

		surfaces.push_back(std::move(result.value()));
		return return_type(std::ref(*std::prev(surfaces.end(), 1)));
	}
};
