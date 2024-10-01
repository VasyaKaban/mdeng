#pragma once

#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include <SDL2/SDL_stdinc.h>
#include <map>
#include <optional>

namespace View
{
    class Window;

    class WindowSubSystem : public hrs::non_copyable
    {
    public:
        WindowSubSystem() noexcept;
        ~WindowSubSystem() noexcept;
        WindowSubSystem(WindowSubSystem&& wss) noexcept;
        WindowSubSystem& operator=(WindowSubSystem&& wss) noexcept;

        bool IsCreated() const noexcept;
        explicit operator bool() const noexcept;

        std::optional<const char*> Init() noexcept;
        void Destroy() noexcept;

        void PollEvents() noexcept;

        hrs::expected<Window*, const char*> CreateWindow(const char* title,
                                                         unsigned int width,
                                                         unsigned int height,
                                                         int x,
                                                         int y,
                                                         Uint32 flags);

        void DropWindow(Window* w) noexcept;
        void DropWindow(Uint32 window_id) noexcept;

        const char* GetError() const noexcept;
    private:
        Window* find_window(Uint32 window_id) noexcept;
    private:
        inline static int counter;

        bool created;
        std::map<Uint32, Window> windows;
    };
};
