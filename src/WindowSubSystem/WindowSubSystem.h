#pragma once

#include <optional>
#include <unordered_map>
#include <vector>
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "WindowSubSystemConfig.h"

namespace View
{
	class Window;

	class WindowSubSystem
		: public hrs::non_copyable,
		  public hrs::non_movable
	{
	private:
		WindowSubSystem() = default;
	public:

		~WindowSubSystem();

		static hrs::expected<WindowSubSystem *, Error> Init() noexcept;

		bool IsInitialized() noexcept;
		void Close() noexcept;

		hrs::expected<Window *, Error> CreateWindow(
			const char *title,
			int x_position,
			int y_position,
			int width,
			int height,
			CreateFlags flags);

		void PollEvents() noexcept;

		std::optional<Error> PeepEvents(int count,
										Event min_type,
										Event max_type,
										bool remove);

		std::optional<Error> PeepEvents(int count,
										std::vector<SDL_Event> &event_storage,
										Event min_type,
										Event max_type,
										bool remove);

		void PumpEvents() noexcept;

		void CallWindowHandlers(const SDL_Event &event) noexcept;

		Window * GetWindowByHandle(WindowHandle handle) noexcept;

		Window * GetWindowByID(WindowID id) noexcept;

	private:
		static WindowSubSystem sub_system;

		static inline auto SUBSYSTEMS_FLAGS = SDL_INIT_EVENTS | SDL_INIT_VIDEO;
	private:
		std::unordered_map<WindowHandle, Window> windows;
	};
};
