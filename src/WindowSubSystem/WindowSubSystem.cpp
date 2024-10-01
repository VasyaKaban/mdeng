#include "WindowSubSystem.h"
#include "Window.h"
#include <SDL2/SDL.h>

namespace View
{
    WindowSubSystem::WindowSubSystem() noexcept
        : created(false)
    {}

    WindowSubSystem::~WindowSubSystem() noexcept
    {
        Destroy();
    }

    WindowSubSystem::WindowSubSystem(WindowSubSystem&& wss) noexcept
        : windows(std::move(wss.windows)),
          created(wss.created)
    {
        if(created)
            counter++;

        wss.Destroy();
    }

    WindowSubSystem& WindowSubSystem::operator=(WindowSubSystem&& wss) noexcept
    {
        Destroy();

        windows = std::move(wss.windows);
        created = wss.created;
        if(created)
            counter++;

        wss.Destroy();

        return *this;
    }

    bool WindowSubSystem::IsCreated() const noexcept
    {
        return created;
    }

    WindowSubSystem::operator bool() const noexcept
    {
        return IsCreated();
    }

    std::optional<const char*> WindowSubSystem::Init() noexcept
    {
        if(IsCreated())
            return {};

        if(counter)
        {
            created = true;
            return {};
        }
        else
        {
            auto res = SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
            if(res == 0)
            {
                counter++;
                created = true;
                return {};
            }
            else
                return SDL_GetError();
        }
    }

    void WindowSubSystem::Destroy() noexcept
    {
        if(!IsCreated())
            return;

        windows.clear();

        counter--;
        if(counter == 0)
            SDL_Quit();
    }

    void WindowSubSystem::PollEvents() noexcept
    {
        if(!IsCreated())
            return;

#define HANDLE_WINDOW_EVENT(EVENT_DATA) \
    { \
        Window* window = find_window(EVENT_DATA.windowID); \
        if(window) \
            window->GetEventHandlers().Handle(EVENT_DATA); \
    }

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_WINDOWEVENT:
                    HANDLE_WINDOW_EVENT(event.window)
                    break;
                case SDL_KEYDOWN:
                    HANDLE_WINDOW_EVENT(event.key);
                    break;
                case SDL_KEYUP:
                    HANDLE_WINDOW_EVENT(event.window);
                    break;
                case SDL_TEXTEDITING:
                    HANDLE_WINDOW_EVENT(event.edit);
                    break;
                case SDL_TEXTEDITING_EXT:
                    HANDLE_WINDOW_EVENT(event.editExt);
                    break;
                case SDL_TEXTINPUT:
                    HANDLE_WINDOW_EVENT(event.text);
                    break;
                case SDL_MOUSEMOTION:
                    HANDLE_WINDOW_EVENT(event.motion);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    HANDLE_WINDOW_EVENT(event.button);
                    break;
                case SDL_MOUSEBUTTONUP:
                    HANDLE_WINDOW_EVENT(event.button);
                    break;
                case SDL_MOUSEWHEEL:
                    HANDLE_WINDOW_EVENT(event.wheel);
                    break;
                case SDL_USEREVENT:
                    HANDLE_WINDOW_EVENT(event.user);
                    break;
            }
        }

#undef HANDLE_WINDOW_EVENT
    }

    hrs::expected<Window*, const char*> WindowSubSystem::CreateWindow(const char* title,
                                                                      unsigned int width,
                                                                      unsigned int height,
                                                                      int x,
                                                                      int y,
                                                                      Uint32 flags)
    {
        if(!IsCreated())
        {
            auto opt_err = Init();
            if(opt_err)
                return opt_err.value();
        }

        SDL_Window* handle = SDL_CreateWindow(title, x, y, width, height, flags);

        if(!handle)
            return SDL_GetError();

        auto window_id = SDL_GetWindowID(handle);
        auto it = windows.insert({window_id, Window(this, handle)});
        return &it.first->second;
    }

    void WindowSubSystem::DropWindow(Window* w) noexcept
    {
        if(!w)
            return;

        DropWindow(SDL_GetWindowID(w->handle));
    }

    void WindowSubSystem::DropWindow(Uint32 window_id) noexcept
    {
        auto it = windows.find(window_id);
        if(it != windows.end())
            windows.erase(it);
    }

    const char* WindowSubSystem::GetError() const noexcept
    {
        if(!IsCreated())
            return nullptr;

        return GetError();
    }

    Window* WindowSubSystem::find_window(Uint32 window_id) noexcept
    {
        auto it = windows.find(window_id);
        if(it != windows.end())
            return &it->second;

        return nullptr;
    }
};
