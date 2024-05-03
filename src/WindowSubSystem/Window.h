#pragma once

#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include "WindowSubSystemConfig.h"
#include "HandlerContainer.hpp"

namespace View
{
	class Window : public hrs::non_copyable
	{
	public:

		struct Resolution
		{
			int width;
			int height;
		};

		struct Position
		{
			int x;
			int y;
		};

		Window() noexcept;
		Window(WindowHandle _handle) noexcept;
		~Window();
		Window(Window &&w) noexcept;
		Window & operator=(Window &&w) noexcept;

		bool IsCreated() const noexcept;
		explicit operator bool() const noexcept;

		void Destroy() noexcept;
		void Resize(const Resolution &resolution) noexcept;
		void SetTitle(const char *title) noexcept;
		void SetBorderState(bool bordered) noexcept;
		void SetResizableState(bool resizable) noexcept;
		void SetPosition(const Position &position) noexcept;
		Resolution GetResolution() const noexcept;
		Resolution GetVulkanSurfaceResolution() const noexcept;

		hrs::expected<std::vector<const char *>, Error> GetVulkanInstanceExtensions() const noexcept;
		hrs::expected<VkSurfaceKHR, Error> CreateVulkanSurface(VkInstance instance) const noexcept;

		void HandleEvent(const SDL_Event &event) const noexcept;
		void HandleEvent(const SDL_WindowEvent &event) const noexcept;
		void HandleEvent(const SDL_KeyboardEvent &event) const noexcept;
		void HandleEvent(const SDL_MouseMotionEvent &event) const noexcept;
		void HandleEvent(const SDL_MouseButtonEvent &event) const noexcept;
		void HandleEvent(const SDL_MouseWheelEvent &event) const noexcept;
		void HandleEvent(const SDL_QuitEvent &event) const noexcept;
		void HandleEvent(const SDL_UserEvent &event) const noexcept;

		WindowEventHandlerContainer & GetWindowEventHandlers() noexcept;
		KeyboardEventHandlerContainer & GetKeyboardEventHandlers() noexcept;
		MouseMotionEventHandlerContainer & GetMouseMotionEventHandlers() noexcept;
		MouseButtonEventHandlerContainer & GetMouseButtonEventHandlers() noexcept;
		MouseWheelEventHandlerContainer & GetMouseWheelEventHandlers() noexcept;
		QuitEventHandlerContainer & GetQuitEventHandlers() noexcept;
		UserEventHandlerContainer & GetUserEventHandlers() noexcept;

		const WindowEventHandlerContainer & GetWindowEventHandlers() const noexcept;
		const KeyboardEventHandlerContainer & GetKeyboardEventHandlers() const noexcept;
		const MouseMotionEventHandlerContainer & GetMouseMotionEventHandlers() const noexcept;
		const MouseButtonEventHandlerContainer & GetMouseButtonEventHandlers() const noexcept;
		const MouseWheelEventHandlerContainer & GetMouseWheelEventHandlers() const noexcept;
		const QuitEventHandlerContainer & GetQuitEventHandlers() const noexcept;
		const UserEventHandlerContainer & GetUserEventHandlers() const noexcept;

	private:
		WindowHandle handle;

		WindowEventHandlerContainer window_event_handlers;
		KeyboardEventHandlerContainer keyboard_event_handlers;
		MouseMotionEventHandlerContainer mouse_motion_event_handlers;
		MouseButtonEventHandlerContainer mouse_button_event_handlers;
		MouseWheelEventHandlerContainer mouse_wheel_event_handlers;
		QuitEventHandlerContainer quit_event_handlers;
		UserEventHandlerContainer user_event_handlers;
	};
};
