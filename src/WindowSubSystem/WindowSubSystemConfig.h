#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

namespace View
{
	using Error = const char *;
	using WindowHandle = SDL_Window *;
	using WindowID = decltype(SDL_GetWindowID(static_cast<SDL_Window *>(nullptr)));
	using CreateFlags = Uint32;

	enum class Event
	{
		WindowEvent = SDL_WINDOWEVENT,
		KeyboardEvent = std::max(SDL_KEYDOWN, SDL_KEYUP),
		MouseMotionEvent = SDL_MOUSEMOTION,
		MouseButtonEvent = std::max(SDL_MOUSEBUTTONDOWN, SDL_MOUSEBUTTONUP),
		MouseWheelEvent = SDL_MOUSEWHEEL,
		QuitEvent = SDL_QUIT,
		UserEvent = SDL_USEREVENT
	};
};
