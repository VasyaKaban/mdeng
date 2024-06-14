#include "dynamic_library.h"

namespace hrs
{
	namespace winapi
	{
		dynamic_library::dynamic_library() noexcept
			: handle(nullptr) {}

		dynamic_library::~dynamic_library()
		{
			close();
		}

		dynamic_library::dynamic_library(dynamic_library &&dl) noexcept
			: handle(std::exchange(dl.handle, nullptr)) {}

		dynamic_library & dynamic_library::operator=(dynamic_library &&dl) noexcept
		{
			close();

			handle = std::exchange(dl.handle, nullptr);

			return *this;
		}

		bool dynamic_library::open(const char *name) noexcept
		{
			if(is_open())
				close();

			handle = LoadLibraryA(name);
			if(!handle)
				return false;

			return true;
		}

		void dynamic_library::close() noexcept
		{
			if(!is_open())
				return;

			handle = (FreeLibrary(handle), nullptr);
		}

		bool dynamic_library::is_open() const noexcept
		{
			return handle != nullptr;
		}

		dynamic_library::operator bool() const noexcept
		{
			return is_open();
		}

		bool dynamic_library::operator==(const dynamic_library &dl) const noexcept
		{
			return handle == dl.handle;
		}

		void * dynamic_library::get_raw_ptr(const char *name) const noexcept
		{
			if(!is_open())
				return nullptr;

			if(!name)
				return nullptr;

			return reinterpret_cast<void *>(GetProcAddress(handle, name));
		}
	};
};
