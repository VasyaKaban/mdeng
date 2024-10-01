#include "Context.h"
#include "hrs/scoped_call.hpp"
#include "hrs/swap_back_pop.hpp"

namespace FireLand
{
    Context::Context(hrs::dynamic_library&& _vk_library,
                     const char* const _vk_libary_name,
                     const GlobalLoader& _global_loader)
        : vk_library(std::move(_vk_library)),
          vk_library_name(_vk_libary_name),
          global_loader(_global_loader),
          instance(VK_NULL_HANDLE)
    {}

    Context::Context() noexcept
        : instance(VK_NULL_HANDLE)
    {}

    Context::~Context()
    {
        Destroy();
    }

    Context::Context(Context&& ctx) noexcept
        : vk_library(std::move(ctx.vk_library)),
          vk_library_name(std::move(ctx.vk_library_name)),
          global_loader(ctx.global_loader),
          instance(std::exchange(ctx.instance, VK_NULL_HANDLE)),
          instance_loader(ctx.instance_loader),
          physical_devices(std::move(ctx.physical_devices)),
          surfaces(std::move(ctx.surfaces)),
          debug_messengers(std::move(ctx.debug_messengers)),
          device_utilizers(std::move(ctx.device_utilizers)),
          allocation_callbacks(ctx.allocation_callbacks)
    {}

    Context& Context::operator=(Context&& ctx) noexcept
    {
        Destroy();

        vk_library = std::move(ctx.vk_library);
        vk_library_name = std::move(ctx.vk_library_name);
        global_loader = ctx.global_loader;
        instance = std::exchange(ctx.instance, VK_NULL_HANDLE);
        instance_loader = ctx.instance_loader;
        physical_devices = std::move(ctx.physical_devices);
        surfaces = std::move(ctx.surfaces);
        debug_messengers = std::move(ctx.debug_messengers);
        device_utilizers = std::move(ctx.device_utilizers);
        allocation_callbacks = ctx.allocation_callbacks;

        return *this;
    }

    hrs::expected<std::uint32_t, VkResult> Context::QueryVersion() const noexcept
    {
        hrs::assert_true_debug(IsInitialized(), "Context (Vulkan library) isn't initialized yet!");

        if(global_loader.vkEnumerateInstanceVersion)
        {
            std::uint32_t version;
            VkResult res = global_loader.vkEnumerateInstanceVersion(&version);
            if(res != VK_SUCCESS)
                return res;

            return version;
        }

        return VK_API_VERSION_1_0;
    }

    hrs::expected<Context, InitResult> Context::Init(std::span<const char* const> library_names)
    {
        hrs::dynamic_library lib;
        InitResult result("vkGetInstanceProcAddr");
        for(const auto& name: library_names)
        {
            bool open_res = lib.open(name);
            if(!open_res)
                continue;

            auto global_vkGetInstanceProcAddr =
                lib.get_ptr<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
            if(!global_vkGetInstanceProcAddr)
                continue;

            GlobalLoader _global_loader;
            auto loader_res = _global_loader.Init(global_vkGetInstanceProcAddr);
            if(loader_res.IsFailure())
            {
                if(loader_res.IsLoaderFunctionFailure())
                    result = "vkGetInstanceProcAddr";
                else
                    result = loader_res.GetRequiredFailureName();

                continue;
            }

            return Context(std::move(lib), name, _global_loader);
        }

        return result;
    }

    std::optional<InitResult> Context::Create(const VkInstanceCreateInfo& info,
                                              const VkAllocationCallbacks* _allocation_callbacks)
    {
        if(IsCreated())
            return {};

        hrs::assert_true_debug(IsInitialized(), "Context (Vulkan library) isn't initialized yet!");

        VkInstance _instance;
        VkResult res = global_loader.vkCreateInstance(&info, _allocation_callbacks, &_instance);
        if(res != VK_SUCCESS)
            return res;

        InstanceLoader _instance_loader;
        auto instance_loader_res =
            _instance_loader.Init(_instance, global_loader.vkGetInstanceProcAddr);
        if(instance_loader_res.IsFailure())
        {
            if(_instance_loader.vkDestroyInstance)
                _instance_loader.vkDestroyInstance(_instance, _allocation_callbacks);
            //else
            //VkInstance LEAK!!!

            if(instance_loader_res.IsLoaderFunctionFailure())
                return InitResult("vkGetInstanceProcAddr");
            else
                return instance_loader_res.GetRequiredFailureName();
        }

        hrs::scoped_call instance_dtor(
            [_instance, &_instance_loader, _allocation_callbacks]()
            {
                _instance_loader.vkDestroyInstance(_instance, _allocation_callbacks);
            });

        std::vector<VkPhysicalDevice> _physical_devices;
        std::uint32_t physical_count = 0;
        res = _instance_loader.vkEnumeratePhysicalDevices(_instance, &physical_count, nullptr);
        if(res != VK_SUCCESS)
            return res;

        _physical_devices.resize(physical_count);
        res = _instance_loader.vkEnumeratePhysicalDevices(_instance,
                                                          &physical_count,
                                                          _physical_devices.data());
        if(res != VK_SUCCESS)
            return res;

        instance = _instance;
        instance_loader = _instance_loader;
        physical_devices = std::move(_physical_devices);
        if(_allocation_callbacks)
            allocation_callbacks = *_allocation_callbacks;

        instance_dtor.drop();
        return {};
    }

    void Context::Destroy() noexcept
    {
        if(IsCreated())
        {
            device_utilizers.clear();
            if(instance_loader.vkDestroyDebugUtilsMessengerEXT)
                for(const auto messenger: debug_messengers)
                    instance_loader.vkDestroyDebugUtilsMessengerEXT(instance,
                                                                    messenger,
                                                                    GetAllocationCallbacks());
            debug_messengers.clear();
            //else
            //	MSG: VkDebugMessengerEXT LEAK!!!
            if(instance_loader.vkDestroySurfaceKHR)
                for(const auto surface: surfaces)
                    instance_loader.vkDestroySurfaceKHR(instance,
                                                        surface,
                                                        GetAllocationCallbacks());
            surfaces.clear();
            //else
            //	MSG: VkSurfaceKHR LEAK!!!
            physical_devices.clear();

            instance_loader.vkDestroyInstance(instance, GetAllocationCallbacks());
            instance = VK_NULL_HANDLE;

            allocation_callbacks.reset();
        }

        if(IsInitialized())
            vk_library.close();
    }

    void Context::AddSurface(VkSurfaceKHR surface)
    {
        if(surface == VK_NULL_HANDLE)
            return;

        surfaces.push_back(surface);
    }

    void Context::DestroySurface(VkSurfaceKHR surface) noexcept
    {
        hrs::assert_true_debug(IsCreated(), "Context isn't created yet!");

        auto it = std::ranges::find_if(surfaces,
                                       [surface](VkSurfaceKHR sur)
                                       {
                                           return sur == surface;
                                       });

        if(it != surfaces.end())
        {
            if(instance_loader.vkDestroySurfaceKHR)
                instance_loader.vkDestroySurfaceKHR(instance, *it, GetAllocationCallbacks());
            //else
            //VkSurfaceKHR LEAK!!!

            hrs::swap_back_pop(surfaces, it);
        }
    }

    void Context::RemoveSurface(VkSurfaceKHR surface) noexcept
    {
        auto it = std::ranges::find_if(surfaces,
                                       [surface](VkSurfaceKHR sur)
                                       {
                                           return sur == surface;
                                       });

        if(it != surfaces.end())
            hrs::swap_back_pop(surfaces, it);
    }

    hrs::expected<VkDebugUtilsMessengerEXT, VkResult>
    Context::CreateDebugMessenger(const VkDebugUtilsMessengerCreateInfoEXT& info)
    {
        hrs::assert_true_debug(IsCreated(), "Context isn't created yet!");

        if(!instance_loader.vkCreateDebugUtilsMessengerEXT)
            return VK_ERROR_INITIALIZATION_FAILED;

        VkDebugUtilsMessengerEXT messenger;
        VkResult res = instance_loader.vkCreateDebugUtilsMessengerEXT(instance,
                                                                      &info,
                                                                      GetAllocationCallbacks(),
                                                                      &messenger);

        if(res != VK_SUCCESS)
            return res;

        debug_messengers.push_back(messenger);
        return messenger;
    }

    void Context::DestroyDebugMessenger(VkDebugUtilsMessengerEXT messenger) noexcept
    {
        hrs::assert_true_debug(IsCreated(), "Context isn't created yet!");
        auto it = std::ranges::find_if(debug_messengers,
                                       [messenger](VkDebugUtilsMessengerEXT mes)
                                       {
                                           return mes == messenger;
                                       });

        if(it != debug_messengers.end())
        {
            if(instance_loader.vkDestroyDebugUtilsMessengerEXT)
                instance_loader.vkDestroyDebugUtilsMessengerEXT(instance,
                                                                *it,
                                                                GetAllocationCallbacks());
            //else
            //vkDestroyDebugUtilsMessengerEXT LEAK!!!

            hrs::swap_back_pop(debug_messengers, it);
        }
    }

    void Context::RemoveDebugMessenger(VkDebugUtilsMessengerEXT messenger) noexcept
    {
        auto it = std::ranges::find_if(debug_messengers,
                                       [messenger](VkDebugUtilsMessengerEXT mes)
                                       {
                                           return mes == messenger;
                                       });

        if(it != debug_messengers.end())
            hrs::swap_back_pop(debug_messengers, it);
    }

    void Context::DestroyDeviceUtilizer(DeviceUtilizer* du) noexcept
    {
        auto it = std::ranges::find_if(device_utilizers,
                                       [du](const std::unique_ptr<DeviceUtilizer>& udu)
                                       {
                                           return du == udu.get();
                                       });

        if(it != device_utilizers.end())
            hrs::swap_back_pop(device_utilizers, it);
    }

    const GlobalLoader& Context::GetGlobalLoader() const noexcept
    {
        return global_loader;
    }

    const InstanceLoader& Context::GetInstanceLoader() const noexcept
    {
        return instance_loader;
    }

    const std::string& Context::GetVulkanLibraryName() const noexcept
    {
        return vk_library_name;
    }

    VkInstance Context::GetHandle() const noexcept
    {
        return instance;
    }

    const std::vector<VkPhysicalDevice>& Context::GetPhysicalDevices() const noexcept
    {
        return physical_devices;
    }

    const std::vector<VkSurfaceKHR>& Context::GetSurfaces() const noexcept
    {
        return surfaces;
    }

    const std::vector<VkDebugUtilsMessengerEXT>& Context::GetDebugMessengers() const noexcept
    {
        return debug_messengers;
    }

    const std::vector<std::unique_ptr<DeviceUtilizer>>& Context::GetDeviceUtilizers() const noexcept
    {
        return device_utilizers;
    }

    std::vector<std::unique_ptr<DeviceUtilizer>>& Context::GetDeviceUtilizers() noexcept
    {
        return device_utilizers;
    }

    const VkAllocationCallbacks* Context::GetAllocationCallbacks() const noexcept
    {
        if(allocation_callbacks)
            return &*allocation_callbacks;

        return nullptr;
    }

    bool Context::IsInitialized() const noexcept
    {
        return vk_library.is_open();
    }

    bool Context::IsCreated() const noexcept
    {
        return instance != VK_NULL_HANDLE;
    }
};
