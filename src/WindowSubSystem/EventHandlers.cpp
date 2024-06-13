#include "EventHandlers.h"

namespace View
{

#define DEFINE_HANDLER(EVENT_TYPE) \
	void EventHandlers::SetHandler(std::function<void (const EVENT_TYPE &)> &&handler) \
	{ \
		EVENT_TYPE##_handler = std::move(handler);\
	} \
	\
	void EventHandlers::Handle(const EVENT_TYPE &event) noexcept \
	{ \
		if(EVENT_TYPE##_handler) \
			EVENT_TYPE##_handler(event); \
	}

	DEFINE_HANDLER(SDL_WindowEvent)
	DEFINE_HANDLER(SDL_KeyboardEvent)
	DEFINE_HANDLER(SDL_TextEditingEvent)
	DEFINE_HANDLER(SDL_TextEditingExtEvent)
	DEFINE_HANDLER(SDL_TextInputEvent)
	DEFINE_HANDLER(SDL_MouseMotionEvent)
	DEFINE_HANDLER(SDL_MouseButtonEvent)
	DEFINE_HANDLER(SDL_MouseWheelEvent)
	DEFINE_HANDLER(SDL_UserEvent)

#undef DEFINE_HANDLER


	void EventHandlers::Handle(const SDL_Event &event) noexcept
	{
		switch(event.type)
		{
			case SDL_WINDOWEVENT:
				Handle(event.window);
				break;
			case SDL_KEYDOWN:
				Handle(event.key);
				break;
			case SDL_KEYUP:
				Handle(event.window);
				break;
			case SDL_TEXTEDITING:
				Handle(event.edit);
				break;
			case SDL_TEXTEDITING_EXT:
				Handle(event.editExt);
				break;
			case SDL_TEXTINPUT:
				Handle(event.text);
				break;
			case SDL_MOUSEMOTION:
				Handle(event.motion);
				break;
			case SDL_MOUSEBUTTONDOWN:
				Handle(event.button);
				break;
			case SDL_MOUSEBUTTONUP:
				Handle(event.button);
				break;
			case SDL_MOUSEWHEEL:
				Handle(event.wheel);
				break;
			case SDL_USEREVENT:
				Handle(event.user);
				break;
		}
	}
};
