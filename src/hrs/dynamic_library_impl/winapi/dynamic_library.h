#pragma once

#include "../../non_creatable.hpp"
#include <libloaderapi,h>

namespace hrs
{
	namespace winapi
	{
		class dynamic_library : hrs::non_copyable
		{
		public:
			using native_handle_t = HMODULE;

			dynamic_library() noexcept;
			~dynamic_library();
			dynamic_library(dynamic_library &&dl) noexcept;
			dynamic_library & operator=(dynamic_library &&dl) noexcept;

			bool open(const char *name) noexcept;
			void close() noexcept;

			bool is_open() const noexcept;
			explicit operator bool() const noexcept;

			void * get_raw_ptr(const char *name) const noexcept;

			template<typename P = void *>
				requires std::is_pointer_v<P>
			P get_ptr(const char *name) const noexcept
			{
				return reinterpret_cast<P>(get_raw_ptr(name));
			}

			template<typename T = void>
			T * get(const char *name) const noexcept
			{
				return reinterpret_cast<T *>(get_raw_ptr(name));
			}

		private:
			native_handle_t handle;
		};
	};
};
