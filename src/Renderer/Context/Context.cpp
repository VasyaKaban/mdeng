#include "Context.h"
#include "../../hrs/iterator_for_each.hpp"

namespace FireLand
{
	Context::Context(vk::Instance _instance, PhysicalDevicesContainer &&_physical_devices) noexcept
		: instance(_instance), physical_devices(std::move(_physical_devices)) {}

	hrs::expected<std::uint32_t, vk::Result> Context::GetVersion() noexcept
	{
		auto version = vk::enumerateInstanceVersion();
		if(version.result != vk::Result::eSuccess)
			return version.result;

		return version.value;
	}

	hrs::expected<std::vector<vk::ExtensionProperties>, vk::Result>
	Context::GetSupportedExtensions(const std::string &layer_name)
	{
		auto extensions = vk::enumerateInstanceExtensionProperties(layer_name);
		if(extensions.result != vk::Result::eSuccess)
			return extensions.result;


		return extensions.value;
	}

	hrs::expected<std::vector<vk::LayerProperties>, vk::Result> Context::GetSupportedLayers()
	{
		auto layers = vk::enumerateInstanceLayerProperties();
		if(layers.result != vk::Result::eSuccess)
			return layers.result;

		return layers.value;
	}

	hrs::expected<Context, vk::Result> Context::Create(const vk::InstanceCreateInfo &info)
	{
		auto inst_opt = vk::createInstanceUnique(info);
		if(inst_opt.result != vk::Result::eSuccess)
			return inst_opt.result;

		auto physical_devs = inst_opt.value.get().enumeratePhysicalDevices();
		if(physical_devs.result != vk::Result::eSuccess)
			return physical_devs.result;

		std::vector<PhysicalDevice> physical_devices;
		physical_devices.reserve(physical_devs.value.size());
		for(auto &ph_dev : physical_devs.value)
			physical_devices.push_back(PhysicalDevice(this, ph_dev));

		return Context(inst_opt.value.release(), std::move(physical_devices));
	}

	Context::~Context()
	{
		Destroy();
	}

	Context::Context(Context &&ctx) noexcept
		: instance(std::exchange(ctx.instance, VK_NULL_HANDLE)),
		  physical_devices(std::move(ctx.physical_devices)),
		  surfaces(std::move(ctx.surfaces)),
		  debug_messengers(ctx.debug_messengers) {}

	Context & Context::operator=(Context &&ctx) noexcept
	{
		Destroy();
		instance = std::exchange(ctx.instance, VK_NULL_HANDLE);
		physical_devices = std::move(ctx.physical_devices);
		surfaces = std::move(ctx.surfaces);
		debug_messengers = std::move(ctx.debug_messengers);
		return *this;
	}

	void Context::Destroy()
	{
		if(instance)
		{
			physical_devices.clear();
			surfaces.clear();

			for(auto &messenger : debug_messengers)
				instance.destroy(messenger);

			debug_messengers.clear();
			instance.destroy();
			instance = VK_NULL_HANDLE;
		}
	}

	bool Context::IsCreated() const noexcept
	{
		return instance;
	}

	const Context::PhysicalDevicesContainer & Context::GetPhysicalDevices() const noexcept
	{
		return physical_devices;
	}

	PhysicalDevice & Context::GetPhysicalDevice(std::size_t index) noexcept
	{
		return physical_devices[index];
	}

	const PhysicalDevice & Context::GetPhysicalDevice(std::size_t index) const noexcept
	{
		return physical_devices[index];
	}

	const Context::SurfacesContainer & Context::GetSurfaces() const noexcept
	{
		return surfaces;
	}

	Surface & Context::GetSurface(std::size_t index) noexcept
	{
		return *std::next(surfaces.begin(), index);
	}

	const Surface & Context::GetSurface(std::size_t index) const noexcept
	{
		return *std::next(surfaces.begin(), index);
	}

	void Context::DropSurface(SurfacesContainer::const_iterator it) noexcept
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(surfaces, it),
							   "Surface is not a part of this context!");

		surfaces.erase(it);
	}

	void Context::DropSurface(const Surface &surface) noexcept
	{
		hrs::iterator_for_each(surfaces, [&](SurfacesContainer::iterator iter) -> bool
		{
			if(iter->GetHandle() == surface.GetHandle())
			{
				surfaces.erase(iter);
				return true;
			}

			return false;
		});
	}

	const Context::DebugMessengersContainer & Context::GetDebugMessengers() const noexcept
	{
		return debug_messengers;
	}

	vk::DebugUtilsMessengerEXT Context::GetDebugMessenger(std::size_t index) const noexcept
	{
		return *std::next(debug_messengers.begin(), index);
	}

	void Context::DropDebugMessenger(vk::DebugUtilsMessengerEXT messenger) noexcept
	{
		hrs::iterator_for_each(debug_messengers, [&](DebugMessengersContainer::iterator iter) -> bool
		{
			if(*iter == messenger)
			{
				instance.destroy(messenger);
				debug_messengers.erase(iter);
				return true;
			}

			return false;
		});
	}

	void Context::DropDebugMessenger(DebugMessengersContainer::const_iterator it) noexcept
	{
		hrs::assert_true_debug(hrs::is_iterator_part_of_range_debug(debug_messengers, it),
							   "Debug messenger is not a part of this context!");

		instance.destroy(*it);
		debug_messengers.erase(it);
	}

	vk::Instance Context::GetHandle() const noexcept
	{
		return instance;
	}

	hrs::expected<vk::DebugUtilsMessengerEXT, vk::Result>
	Context::CreateDebugMessenger(const vk::DebugUtilsMessengerCreateInfoEXT &info) noexcept
	{
		auto [messenger_res, messenger] = instance.createDebugUtilsMessengerEXT(info);
		if(messenger_res != vk::Result::eSuccess)
			return messenger_res;

		debug_messengers.push_back(messenger);
		return *std::prev(debug_messengers.end());
	}
};
