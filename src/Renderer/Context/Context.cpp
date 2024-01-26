#include "Context.h"
#include "../../hrs/iterator_for_each.hpp"

namespace FireLand
{
	Context::Context(vk::Instance &&_instance, std::vector<PhysicalDevice> &&_physical_devices) noexcept
		: instance(std::move(_instance)), physical_devices(std::move(_physical_devices)) {}

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
			physical_devices.push_back(ph_dev);

		return Context(std::move(inst_opt.value.release()), std::move(physical_devices));
	}

	Context::~Context()
	{
		Destroy();
	}

	Context::Context(Context &&ctx) noexcept
		: instance(std::move(ctx.instance)), physical_devices(std::move(ctx.physical_devices))
	{
		ctx.instance = VK_NULL_HANDLE;
	}

	Context & Context::operator=(Context &&ctx) noexcept
	{
		Destroy();
		instance = std::move(ctx.instance);
		physical_devices = std::move(ctx.physical_devices);
		ctx.instance = VK_NULL_HANDLE;
		return *this;
	}

	void Context::Destroy()
	{
		if(instance)
		{
			physical_devices.clear();
			for(auto &surface : surfaces)
				surface.Destroy(instance);

			surfaces.clear();
			instance.destroy();
		}
	}

	bool Context::IsCreated() const noexcept
	{
		return instance;
	}

	const std::vector<PhysicalDevice> & Context::GetPhysicalDevices() const noexcept
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

	const std::list<Surface> & Context::GetSurfaces() const noexcept
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

	void Context::DropSurface(const Surface &surface) noexcept
	{
		hrs::iterator_for_each(surfaces, [&](std::list<Surface>::iterator iter) -> bool
		{
			if(iter->GetSurface() == surface.GetSurface())
			{
				iter->Destroy(instance);
				surfaces.erase(iter);
				return true;
			}

			return false;
		});
	}

	vk::Instance Context::GetInstance() const noexcept
	{
		return instance;
	}
};
