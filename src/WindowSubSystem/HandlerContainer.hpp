#pragma once

#include <functional>
#include <map>
#include <cstdint>
#include <cassert>
#include "WindowSubSystemConfig.h"

namespace View
{
	template<typename H>
	class HandlerContainer
	{
	public:
		HandlerContainer() = default;
		~HandlerContainer() = default;
		HandlerContainer(const HandlerContainer &) = default;
		HandlerContainer(HandlerContainer &&) = default;
		HandlerContainer & operator=(const HandlerContainer &) = default;
		HandlerContainer & operator=(HandlerContainer &&) = default;

		template<typename F>
		std::uint64_t AddHandler(F &&f)
		{
			std::uint64_t id = get_free_id();
			handlers.insert({id, std::forward<F>(f)});
			return id;
		}

		bool RemoveHandler(std::uint64_t id) noexcept
		{
			auto it = handlers.find(id);
			if(it == handlers.end())
				return false;

			handlers.erase(it);
			return true;
		}

		bool HasHandler(std::uint64_t id) const noexcept
		{
			auto it = handlers.find(id);
			return it != handlers.end();
		}

		std::function<H> * GetHandler(std::uint64_t id) noexcept
		{
			auto it = handlers.find(id);
			return (it == handlers.end() ? nullptr : &it->second);
		}

		const std::function<H> * GetHandler(std::uint64_t id) const noexcept
		{
			auto it = handlers.find(id);
			return (it == handlers.end() ? nullptr : &it->second);
		}

		std::map<std::uint64_t, std::function<H>> & GetHandlers() noexcept
		{
			return handlers;
		}

		const std::map<std::uint64_t, std::function<H>> & GetHandlers() const noexcept
		{
			return handlers;
		}

		void Clear() noexcept
		{
			handlers.clear();
		}

	private:

		std::uint64_t get_free_id() const noexcept
		{
			if(handlers.empty())
				return 0;

			auto it = handlers.begin();//min
			if(it->first != 0)
				return it->first - 1;

			auto prev_it = handlers.begin();
			for(it = std::next(handlers.begin()); it != handlers.end(); prev_it = it++)
			{
				if(it->first != prev_it->first + 1)
					return it->first + 1;
			}

			it = std::prev(handlers.end());//max
			if(it->first != std::numeric_limits<std::uint64_t>::max())
				return it->first + 1;

			assert(false);
		}

	private:
		std::map<std::uint64_t, std::function<H>> handlers;
	};

	using WindowEventHandler = void (const SDL_WindowEvent &);
	using KeyboardEventHandler = void (const SDL_KeyboardEvent &);
	using MouseMotionEventHandler = void (const SDL_MouseMotionEvent &);
	using MouseButtonEventHandler = void (const SDL_MouseButtonEvent &);
	using MouseWheelEventHandler = void (const SDL_MouseWheelEvent &);
	using QuitEventHandler = void (const SDL_QuitEvent &);
	using UserEventHandler = void (const SDL_UserEvent &);

	using WindowEventHandlerContainer = HandlerContainer<WindowEventHandler>;
	using KeyboardEventHandlerContainer = HandlerContainer<KeyboardEventHandler>;
	using MouseMotionEventHandlerContainer = HandlerContainer<MouseMotionEventHandler>;
	using MouseButtonEventHandlerContainer = HandlerContainer<MouseButtonEventHandler>;
	using MouseWheelEventHandlerContainer = HandlerContainer<MouseWheelEventHandler>;
	using QuitEventHandlerContainer = HandlerContainer<QuitEventHandler>;
	using UserEventHandlerContainer = HandlerContainer<UserEventHandler>;

};
