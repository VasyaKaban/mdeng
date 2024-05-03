#include "WindowSubSystem.h"
#include "Window.h"

#include <iostream>

namespace View
{
	WindowSubSystem WindowSubSystem::sub_system{};

	WindowSubSystem::~WindowSubSystem()
	{
		Close();
	}

	hrs::expected<WindowSubSystem *, Error> WindowSubSystem::Init() noexcept
	{
		if(sub_system.IsInitialized())
			return &sub_system;

		auto res = SDL_Init(SUBSYSTEMS_FLAGS);
		if(res != 0)
			return SDL_GetError();

		return &sub_system;
	}

	bool WindowSubSystem::IsInitialized() noexcept
	{
		return SUBSYSTEMS_FLAGS & SDL_WasInit(SUBSYSTEMS_FLAGS);
	}

	void WindowSubSystem::Close() noexcept
	{
		windows.clear();
		SDL_Quit();
	}

	hrs::expected<Window * , Error> WindowSubSystem::CreateWindow(
		const char *title,
		int x_position,
		int y_position,
		int width,
		int height,
		CreateFlags flags)
	{
		SDL_Window *handle = SDL_CreateWindow(title, x_position, y_position, width, height, flags);
		if(!handle)
			return Error(SDL_GetError());

		auto it = windows.insert({handle, Window(handle)});

		return &it.first->second;
	}

	void WindowSubSystem::PollEvents() noexcept
	{
		SDL_Event event;
		while(SDL_PollEvent(&event))
			CallWindowHandlers(event);
	}

	std::optional<Error> WindowSubSystem::PeepEvents(int count,
													 Event min_type,
													 Event max_type,
													 bool remove)
	{
		if(count == 0)
			return {};

		std::vector<SDL_Event> events;
		events.resize(count);
		int res = SDL_PeepEvents(events.data(),
								 count,
								 (remove ? SDL_GETEVENT : SDL_PEEKEVENT),
								 static_cast<Uint32>(min_type),
								 static_cast<Uint32>(max_type));

		if(res < 0)
			return Error(SDL_GetError());

		for(int i = 0; i < res; i++)
			CallWindowHandlers(events[i]);

		return {};
	}

	std::optional<Error> WindowSubSystem::PeepEvents(int count,
													 std::vector<SDL_Event> &event_storage,
													 Event min_type,
													 Event max_type,
													 bool remove)
	{
		if(count == 0)
			return {};

		if(event_storage.size() < count)
			event_storage.resize(count);

		int res = SDL_PeepEvents(event_storage.data(),
								 count,
								 (remove ? SDL_GETEVENT : SDL_PEEKEVENT),
								 static_cast<Uint32>(min_type),
								 static_cast<Uint32>(max_type));

		if(res < 0)
			return Error(SDL_GetError());

		for(int i = 0; i < res; i++)
			CallWindowHandlers(event_storage[i]);

		return {};

	}

	void WindowSubSystem::PumpEvents() noexcept
	{
		SDL_PumpEvents();
	}

	void WindowSubSystem::CallWindowHandlers(const SDL_Event &event) noexcept
	{
		switch(event.type)
		{
			case SDL_WINDOWEVENT:
				{
					Window *window = GetWindowByID(event.window.windowID);
					if(window)
						window->HandleEvent(event.window);
				}
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					Window *window = GetWindowByID(event.key.windowID);
					if(window)
						window->HandleEvent(event.key);
				}
				break;
			case SDL_MOUSEMOTION:
				{
					Window *window = GetWindowByID(event.motion.windowID);
					if(window)
						window->HandleEvent(event.motion);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				{
					Window *window = GetWindowByID(event.button.windowID);
					if(window)
						window->HandleEvent(event.button);
				}
				break;
			case SDL_MOUSEWHEEL:
				{
					Window *window = GetWindowByID(event.wheel.windowID);
					if(window)
						window->HandleEvent(event.wheel);
				}
				break;
			case SDL_QUIT:
				{
					for(auto &window : windows)
						window.second.HandleEvent(event.quit);
				}
				break;
			case SDL_USEREVENT:
				{
					Window *window = GetWindowByID(event.user.windowID);
					if(window)
						window->HandleEvent(event.user);
				}
				break;
		}
	}

	Window * WindowSubSystem::GetWindowByHandle(WindowHandle handle) noexcept
	{
		if(handle == nullptr)
			return nullptr;

		auto it = windows.find(handle);
		if(it == windows.end())
			return nullptr;

		return &it->second;
	}

	Window * WindowSubSystem::GetWindowByID(WindowID id) noexcept
	{
		return GetWindowByHandle(SDL_GetWindowFromID(id));
	}
};
