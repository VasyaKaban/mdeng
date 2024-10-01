#pragma once

#include "EventHandlers.h"
#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_video.h>
#include <span>
#include <string>

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

namespace View
{
    class WindowSubSystem;

    class Window
    {
    private:
        friend class WindowSubSystem;

        Window(WindowSubSystem* _parent_sub_system, SDL_Window* _handle) noexcept;
    public:
        Window() noexcept;
        ~Window();
        Window(Window&& w) noexcept;
        Window& operator=(Window&& w) noexcept;

        void Destroy() noexcept;

        bool IsCreated() const noexcept;
        explicit operator bool() const noexcept;

        std::span<const char*> EnumerateVulkanExtensions() const;
        const char* GetVulkanSurfaceCreateFunctionName() const noexcept;

        hrs::expected<VkSurfaceKHR, VkResult>
        CreateSurface(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
                      VkInstance instance,
                      const VkAllocationCallbacks* allocation_callbacks) const noexcept;

        std::pair<unsigned int, unsigned int> GetResolution() const noexcept;
        void SetResolution(unsigned int width, unsigned int height) noexcept;

        std::string GetTitle() const;
        void SetTitle(const char* title) noexcept;

        void WarpMouse(int x, int y) noexcept;

        void SetFullscreenState(Uint32 flags) noexcept;

        hrs::expected<SDL_SysWMinfo, const char*> GetWindowManagerInfo() const noexcept;

        EventHandlers& GetEventHandlers() noexcept;
        const EventHandlers& GetEventHandlers() const noexcept;

        WindowSubSystem* GetParentSubSystem() noexcept;
        const WindowSubSystem* GetParentSubSystem() const noexcept;

        Uint32 GetID() const noexcept;
    private:
        const char* get_vulkan_surface_create_function_name(SDL_SYSWM_TYPE type) const noexcept;
    private:
        WindowSubSystem* parent_sub_system;
        SDL_Window* handle;
        EventHandlers event_handlers;
    };
};
