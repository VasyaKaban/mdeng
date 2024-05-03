#include "Window.h"
#include <vulkan/vulkan_core.h>

namespace View
{
	Window::Window() noexcept
		: handle(nullptr) {}

	Window::Window(WindowHandle _handle) noexcept
		: handle(_handle) {}

	Window::~Window()
	{
		Destroy();
	}

	Window::Window(Window &&w) noexcept
		: handle(std::exchange(w.handle, nullptr)),
		  window_event_handlers(std::move(w.window_event_handlers)),
		  keyboard_event_handlers(std::move(w.keyboard_event_handlers)),
		  mouse_motion_event_handlers(std::move(w.mouse_motion_event_handlers)),
		  mouse_button_event_handlers(std::move(w.mouse_button_event_handlers)),
		  mouse_wheel_event_handlers(std::move(w.mouse_wheel_event_handlers)),
		  quit_event_handlers(std::move(w.quit_event_handlers)),
		  user_event_handlers(std::move(w.user_event_handlers)) {}

	Window & Window::operator=(Window &&w) noexcept
	{
		Destroy();

		handle = std::exchange(w.handle, nullptr);
		window_event_handlers = std::move(w.window_event_handlers);
		keyboard_event_handlers = std::move(w.keyboard_event_handlers);
		mouse_motion_event_handlers = std::move(w.mouse_motion_event_handlers);
		mouse_button_event_handlers = std::move(w.mouse_button_event_handlers);
		mouse_wheel_event_handlers = std::move(w.mouse_wheel_event_handlers);
		quit_event_handlers = std::move(w.quit_event_handlers);
		user_event_handlers = std::move(w.user_event_handlers);

		return *this;
	}

	bool Window::IsCreated() const noexcept
	{
		return handle;
	}

	Window::operator bool() const noexcept
	{
		return IsCreated();
	}

	void Window::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		window_event_handlers.Clear();
		keyboard_event_handlers.Clear();
		mouse_motion_event_handlers.Clear();
		mouse_button_event_handlers.Clear();
		mouse_wheel_event_handlers.Clear();
		quit_event_handlers.Clear();
		user_event_handlers.Clear();

		SDL_DestroyWindow(handle);
		handle = nullptr;
	}

	void Window::Resize(const Resolution &resolution) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowSize(handle, resolution.width, resolution.height);
	}

	void Window::SetTitle(const char *title) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowTitle(handle, title);
	}

	void Window::SetBorderState(bool bordered) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowBordered(handle, static_cast<SDL_bool>(bordered));
	}

	void Window::SetResizableState(bool resizable) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowResizable(handle, static_cast<SDL_bool>(resizable));
	}

	void Window::SetPosition(const Position &position) noexcept
	{
		if(!IsCreated())
			return;

		SDL_SetWindowPosition(handle, position.x, position.y);
	}

	Window::Resolution Window::GetResolution() const noexcept
	{
		Resolution res;
		if(IsCreated())
			SDL_GetWindowSize(handle, &res.width, &res.height);

		return res;
	}

	Window::Resolution Window::GetVulkanSurfaceResolution() const noexcept
	{
		Resolution res;
		if(IsCreated())
			SDL_Vulkan_GetDrawableSize(handle, &res.width, &res.height);

		return res;
	}

	hrs::expected<std::vector<const char *>, Error> Window::GetVulkanInstanceExtensions() const noexcept
	{
		std::vector<const char *> names;
		if(IsCreated())
		{
			unsigned int count;
			auto res = SDL_Vulkan_GetInstanceExtensions(handle, &count, nullptr);
			if(res == SDL_FALSE)
				return Error(SDL_GetError());

			names.resize(count);
			res = SDL_Vulkan_GetInstanceExtensions(handle, &count, names.data());
			if(res == SDL_FALSE)
				return Error(SDL_GetError());
		}

		return names;
	}

	hrs::expected<VkSurfaceKHR, Error> Window::CreateVulkanSurface(VkInstance instance) const noexcept
	{
		if(!IsCreated())
			return VkSurfaceKHR(VK_NULL_HANDLE);

		VkSurfaceKHR surface;
		auto res = SDL_Vulkan_CreateSurface(handle, instance, &surface);
		if(res == SDL_FALSE)
			return Error(SDL_GetError());

		return surface;
	}

	void Window::HandleEvent(const SDL_Event &event) const noexcept
	{
		switch(event.type)
		{
			case SDL_WINDOWEVENT:
				HandleEvent(event.window);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				HandleEvent(event.key);
				break;
			case SDL_MOUSEMOTION:
				HandleEvent(event.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				HandleEvent(event.button);
				break;
			case SDL_MOUSEWHEEL:
				HandleEvent(event.wheel);
				break;
			case SDL_QUIT:
				HandleEvent(event.quit);
				break;
			case SDL_USEREVENT:
				HandleEvent(event.user);
				break;
		}
	}

	void Window::HandleEvent(const SDL_WindowEvent &event) const noexcept
	{
		for(auto &[_, handler] : window_event_handlers.GetHandlers())
			handler(event);
	}

	void Window::HandleEvent(const SDL_KeyboardEvent &event) const noexcept
	{
		for(auto &[_, handler] : keyboard_event_handlers.GetHandlers())
			handler(event);
	}

	void Window::HandleEvent(const SDL_MouseMotionEvent &event) const noexcept
	{
		for(auto &[_, handler] : mouse_motion_event_handlers.GetHandlers())
			handler(event);
	}

	void Window::HandleEvent(const SDL_MouseButtonEvent &event) const noexcept
	{
		for(auto &[_, handler] : mouse_button_event_handlers.GetHandlers())
			handler(event);
	}

	void Window::HandleEvent(const SDL_MouseWheelEvent &event) const noexcept
	{
		for(auto &[_, handler] : mouse_wheel_event_handlers.GetHandlers())
			handler(event);
	}

	void Window::HandleEvent(const SDL_QuitEvent &event) const noexcept
	{
		for(auto &[_, handler] : quit_event_handlers.GetHandlers())
			handler(event);
	}

	void Window::HandleEvent(const SDL_UserEvent &event) const noexcept
	{
		for(auto &[_, handler] : user_event_handlers.GetHandlers())
			handler(event);
	}

	WindowEventHandlerContainer & Window::GetWindowEventHandlers() noexcept
	{
		return window_event_handlers;
	}

	KeyboardEventHandlerContainer & Window::GetKeyboardEventHandlers() noexcept
	{
		return keyboard_event_handlers;
	}

	MouseMotionEventHandlerContainer & Window::GetMouseMotionEventHandlers() noexcept
	{
		return mouse_motion_event_handlers;
	}

	MouseButtonEventHandlerContainer & Window::GetMouseButtonEventHandlers() noexcept
	{
		return mouse_button_event_handlers;
	}

	MouseWheelEventHandlerContainer & Window::GetMouseWheelEventHandlers() noexcept
	{
		return mouse_wheel_event_handlers;
	}

	QuitEventHandlerContainer & Window::GetQuitEventHandlers() noexcept
	{
		return quit_event_handlers;
	}

	UserEventHandlerContainer & Window::GetUserEventHandlers() noexcept
	{
		return user_event_handlers;
	}

	const WindowEventHandlerContainer & Window::GetWindowEventHandlers() const noexcept
	{
		return window_event_handlers;
	}

	const KeyboardEventHandlerContainer & Window::GetKeyboardEventHandlers() const noexcept
	{
		return keyboard_event_handlers;
	}

	const MouseMotionEventHandlerContainer & Window::GetMouseMotionEventHandlers() const noexcept
	{
		return mouse_motion_event_handlers;
	}

	const MouseButtonEventHandlerContainer & Window::GetMouseButtonEventHandlers() const noexcept
	{
		return mouse_button_event_handlers;
	}

	const MouseWheelEventHandlerContainer & Window::GetMouseWheelEventHandlers() const noexcept
	{
		return mouse_wheel_event_handlers;
	}

	const QuitEventHandlerContainer & Window::GetQuitEventHandlers() const noexcept
	{
		return quit_event_handlers;
	}

	const UserEventHandlerContainer & Window::GetUserEventHandlers() const noexcept
	{
		return user_event_handlers;
	}
};
