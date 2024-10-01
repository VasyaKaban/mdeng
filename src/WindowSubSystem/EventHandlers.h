#pragma once

#include <SDL2/SDL_events.h>
#include <functional>

namespace View
{
    template<typename E>
    class EventHandler
    {
    public:
        using handler_t = std::function<void(const E&)>;

        EventHandler() = default;
        ~EventHandler() = default;
        EventHandler(const EventHandler&) = default;
        EventHandler(EventHandler&&) = default;
        EventHandler& operator=(const EventHandler&) = default;
        EventHandler& operator=(EventHandler&&) = default;

        EventHandler(const handler_t& _handler)
            : handler(_handler)
        {}

        EventHandler(handler_t&& _handler)
            : handler(_handler)
        {}

        EventHandler& operator=(const handler_t& _handler)
        {
            handler = _handler;
            return *this;
        }

        EventHandler& operator=(handler_t&& _handler)
        {
            handler = std::move(_handler);
            return *this;
        }

        bool IsCreated() const noexcept
        {
            return handler;
        }

        explicit operator bool() const noexcept
        {
            return IsCreated();
        }

        void Handle(const E& event) noexcept
        {
            if(!IsCreated())
                return;

            handler(event);
        }
    protected:
        std::function<void(const E&)> handler;
    };

#define DECLARE_HANDLER(EVENT_TYPE) \
private: \
    std::function<void(const EVENT_TYPE&)> EVENT_TYPE##_handler; \
public: \
    void SetHandler(std::function<void(const EVENT_TYPE&)>&& handler); \
    void Handle(const EVENT_TYPE& event) noexcept;

    class EventHandlers
    {
    public:
        EventHandlers() = default;
        ~EventHandlers() = default;
        EventHandlers(const EventHandlers&) = default;
        EventHandlers(EventHandlers&&) = default;
        EventHandlers& operator=(const EventHandlers&) = default;
        EventHandlers& operator=(EventHandlers&&) = default;

        void Handle(const SDL_Event& event) noexcept;

        DECLARE_HANDLER(SDL_WindowEvent)
        DECLARE_HANDLER(SDL_KeyboardEvent)
        DECLARE_HANDLER(SDL_TextEditingEvent)
        DECLARE_HANDLER(SDL_TextEditingExtEvent)
        DECLARE_HANDLER(SDL_TextInputEvent)
        DECLARE_HANDLER(SDL_MouseMotionEvent)
        DECLARE_HANDLER(SDL_MouseButtonEvent)
        DECLARE_HANDLER(SDL_MouseWheelEvent)
        DECLARE_HANDLER(SDL_UserEvent)
    };

#undef DECLARE_HANDLER
};
