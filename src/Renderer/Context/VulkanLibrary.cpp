#include <dlfcn.h>
#include <cstring>
#include <cassert>
#include "VulkanLibrary.h"

namespace FireLand
{
	VulkanLibrary::VulkanLibrary() noexcept
		: lib({}) {}

	VulkanLibrary::~VulkanLibrary()
	{
		Close();
	}

	VulkanLibrary::VulkanLibrary(VulkanLibrary &&vl) noexcept
		: lib(std::move(vl.lib)),
		  loader(vl.loader) {}

	VulkanLibrary & VulkanLibrary::operator=(VulkanLibrary &&vl) noexcept
	{
		Close();

		lib = std::move(vl.lib);
		loader = vl.loader;

		return *this;
	}

	void VulkanLibrary::Close() noexcept
	{
		contexts = decltype(contexts){};
		lib.close();
	}

	std::optional<std::size_t>
	VulkanLibrary::Open(std::span<const char * const> names) noexcept
	{
		if(names.empty())
			return {};

		if(IsOpen())
			Close();

		for(std::size_t i = 0; i < names.size(); i++)
		{
			if(names[i])
			{
				bool open_res = lib.open(names[i]);
				if(open_res)
				{
					if(loader.Init(lib.get_ptr<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr")))
						return i;
					else
						lib.close();
				}
			}
		}

		return {};
	}

	hrs::expected<Context *, VkResult>
	VulkanLibrary::CreateContext(const VkInstanceCreateInfo &info,
								 std::shared_ptr<VkAllocationCallbacks> _allocation_cbacks)
	{
		if(!IsOpen())
			return VK_ERROR_INITIALIZATION_FAILED;

		VkInstance instance;
		VkResult res = loader.CreateInstance(&info, _allocation_cbacks.get(), &instance);
		if(res != VK_SUCCESS)
			return res;

		Context ctx;
		res = ctx.Init(this, instance, _allocation_cbacks);
		if(res != VK_SUCCESS)
		{
#warning POTENTIAL LEAK!!!
			//loader.DestroyInstance(instance, _allocation_cbacks.get());
			return res;
		}

		contexts.push_back(std::move(ctx));
		return &contexts.back();
	}

	void VulkanLibrary::DestroyContext(const Context &ctx) noexcept
	{
		if(auto it = std::ranges::find(contexts, ctx); it != contexts.end())
			contexts.erase(it);
	}

	bool VulkanLibrary::IsOpen() const noexcept
	{
		return lib.is_open();
	}

	VulkanLibrary::operator bool() const noexcept
	{
		return IsOpen();
	}

	PFN_vkGetInstanceProcAddr VulkanLibrary::GetResolver() const noexcept
	{
		if(IsOpen())
			return loader.GetInstanceProcAddr;

		return nullptr;
	}

	PFN_vkVoidFunction VulkanLibrary::GetProcAddressRaw(const char * const name) const noexcept
	{
		if(IsOpen())
			return loader.GetInstanceProcAddr(VK_NULL_HANDLE, name);

		return nullptr;
	}

	const GlobalLoader & VulkanLibrary::GetLoader() const noexcept
	{
		return loader;
	}

	bool VulkanLibrary::operator==(const VulkanLibrary &vl) const noexcept
	{
		return lib == vl.lib;
	}
};

