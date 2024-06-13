#include "Window.h"
#include "WindowSubSystem.h"

#ifdef SDL_VIDEO_DRIVER_WINDOWS
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#endif
#ifdef SDL_VIDEO_DRIVER_X11
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif
#ifdef SDL_VIDEO_DRIVER_WAYLAND
#include <vulkan/vulkan_wayland.h>
#endif

namespace View
{
	Window::Window(WindowSubSystem *_parent_sub_system,
				   SDL_Window *_handle) noexcept
		: parent_sub_system(_parent_sub_system),
		  handle(_handle) {}

	Window::Window() noexcept
		: parent_sub_system(nullptr),
		  handle(nullptr) {}

	Window::~Window()
	{
		Destroy();
	}

	Window::Window(Window &&w) noexcept
		: parent_sub_system(std::exchange(w.parent_sub_system, nullptr)),
		  handle(std::exchange(w.handle, nullptr)),
		  event_handlers(std::move(w.event_handlers)) {}

	Window & Window::operator=(Window &&w) noexcept
	{
		Destroy();

		parent_sub_system = std::exchange(w.parent_sub_system, nullptr);
		handle = std::exchange(w.handle, nullptr);
		event_handlers = std::move(w.event_handlers);

		return *this;
	}

	void Window::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		handle = (SDL_DestroyWindow(handle), nullptr);
		parent_sub_system = nullptr;
	}

	bool Window::IsCreated() const noexcept
	{
		return handle;
	}

	Window::operator bool() const noexcept
	{
		return IsCreated();
	}

	std::span<const char *> Window::EnumerateVulkanExtensions() const
	{
		auto sys_wm_type_exp = GetWindowManagerInfo();
		if(!sys_wm_type_exp)
			return {};

		switch(sys_wm_type_exp->subsystem)
		{
#ifdef SDL_VIDEO_DRIVER_WINDOWS
			case SDL_SYSWM_TYPE::SDL_SYSWM_WINDOWS:
				{
					static std::array extensions =
					{
						VK_KHR_SURFACE_EXTENSION_NAME,
						VK_KHR_WIN32_SURFACE_EXTENSION_NAME
					};

					return extensions;
				}
				break;
#endif
#ifdef SDL_VIDEO_DRIVER_X11
			case SDL_SYSWM_TYPE::SDL_SYSWM_X11:
				{
					static std::array extensions =
					{
						VK_KHR_SURFACE_EXTENSION_NAME,
						VK_KHR_XLIB_SURFACE_EXTENSION_NAME
					};

					return extensions;
				}
				break;
#endif
#ifdef SDL_VIDEO_DRIVER_WAYLAND
			case SDL_SYSWM_TYPE::SDL_SYSWM_WAYLAND:
				{
					static std::array extensions =
					{
						VK_KHR_SURFACE_EXTENSION_NAME,
						VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
					};

					return extensions;
				}
				break;
#endif
			default:
				return {};
				break;
		}
	}

	hrs::expected<VkSurfaceKHR, VkResult>
	Window::CreateSurface(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr,
						  VkInstance instance,
						  const VkAllocationCallbacks *allocation_callbacks) const noexcept
	{
		if(instance == VK_NULL_HANDLE || !vkGetInstanceProcAddr)
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;

		auto sys_wm_info_exp = GetWindowManagerInfo();
		if(!sys_wm_info_exp)
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;

		auto func_name = get_vulkan_surface_create_function_name(sys_wm_info_exp->subsystem);
		if(!func_name)
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;

		auto func = vkGetInstanceProcAddr(instance, func_name);
		if(!func)
			return VkResult::VK_ERROR_INITIALIZATION_FAILED;

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		switch(sys_wm_info_exp->subsystem)
		{
#ifdef SDL_VIDEO_DRIVER_WINDOWS
			case SDL_SYSWM_TYPE::SDL_SYSWM_WINDOWS:
				{
					auto vkCreateSurface = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(func);
					const VkWin32SurfaceCreateInfoKHR info =
					{
						.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
						.pNext = nullptr,
						.flags = {},
						.hinstance = sys_wm_info_exp->info.win.hinstance,
						.hwnd = sys_wm_info_exp->info.win.window
					};

					auto res = vkCreateSurface(instance, &info, allocation_callbacks, &surface);
					if(res != VkResult::VK_SUCCESS)
						return res;
				}
				break;
#endif
#ifdef SDL_VIDEO_DRIVER_X11
			case SDL_SYSWM_TYPE::SDL_SYSWM_X11:
				{
					auto vkCreateSurface = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(func);
					const VkXlibSurfaceCreateInfoKHR info =
						{
							.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
							.pNext = nullptr,
							.flags = {},
							.dpy = sys_wm_info_exp->info.x11.display,
							.window = sys_wm_info_exp->info.x11.window
						};

					auto res = vkCreateSurface(instance, &info, allocation_callbacks, &surface);
					if(res != VkResult::VK_SUCCESS)
						return res;
				}
				break;
#endif
#ifdef SDL_VIDEO_DRIVER_WAYLAND
			case SDL_SYSWM_TYPE::SDL_SYSWM_WAYLAND:
				{
					auto vkCreateSurface = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(func);
					const VkWaylandSurfaceCreateInfoKHR info =
					{
						.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
						.pNext = nullptr,
						.flags = {},
						.display = sys_wm_info_exp->info.wl.display,
						.surface = sys_wm_info_exp->info.wl.surface
					};

					auto res = vkCreateSurface(instance, &info, allocation_callbacks, &surface);
					if(res != VkResult::VK_SUCCESS)
						return res;
				}
				break;
#endif
			default:
				return VkResult::VK_ERROR_INITIALIZATION_FAILED;
				break;
		}

		return surface;
	}

	std::pair<unsigned int, unsigned int> Window::GetResolution() const noexcept
	{
		if(!IsCreated())
			return {0, 0};

		int w, h;
		SDL_GetWindowSize(handle, &w, &h);
		return {w, h};
	}

	void Window::SetResolution(unsigned int width, unsigned int height) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowSize(handle, width, height);
	}

	std::string Window::GetTitle() const
	{
		if(!IsCreated())
			return "";

		return SDL_GetWindowTitle(handle);
	}

	void Window::SetTitle(const char *title) noexcept
	{
		if(!IsCreated())
			return;


		SDL_SetWindowTitle(handle, title);
	}

	void Window::WarpMouse(int x, int y) noexcept
	{
		if(!IsCreated())
			return;

		SDL_WarpMouseInWindow(handle, x, y);
	}

	void Window::SetFullscreenState(Uint32 flags) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowFullscreen(handle, flags);
	}

	hrs::expected<SDL_SysWMinfo, const char *> Window::GetWindowManagerInfo() const noexcept
	{
		SDL_SysWMinfo info;
		SDL_VERSION(&info.version)

		if(!IsCreated())
			return "Bad window handle!";//-> assert maybe?

		auto res = SDL_GetWindowWMInfo(handle, &info);
		if(res == SDL_FALSE)
			return SDL_GetError();

		return info;
	}

	EventHandlers & Window::GetEventHandlers() noexcept
	{
		return event_handlers;
	}

	const EventHandlers & Window::GetEventHandlers() const noexcept
	{
		return event_handlers;
	}

	WindowSubSystem * Window::GetParentSubSystem() noexcept
	{
		return parent_sub_system;
	}

	const WindowSubSystem * Window::GetParentSubSystem() const noexcept
	{
		return parent_sub_system;
	}

	Uint32 Window::GetID() const noexcept
	{
		if(!IsCreated())
			return 0;

		return SDL_GetWindowID(handle);
	}

	const char * Window::get_vulkan_surface_create_function_name(SDL_SYSWM_TYPE type) const noexcept
	{
		switch(type)
		{
			case SDL_SYSWM_TYPE::SDL_SYSWM_WINDOWS:
				return "vkCreateWin32SurfaceKHR";
				break;
			case SDL_SYSWM_TYPE::SDL_SYSWM_X11:
				return "vkCreateXlibSurfaceKHR";
				break;
			case SDL_SYSWM_TYPE::SDL_SYSWM_WAYLAND:
				return "vkCreateWaylandSurfaceKHR";
				break;
			default:
				return nullptr;
				break;
		}
	}
};

