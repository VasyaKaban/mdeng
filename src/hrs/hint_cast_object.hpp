#pragma once

#include <concepts>

namespace hrs
{
	template<typename E>
	struct hint_traits
	{
		constexpr static void hint() noexcept {};
	};

	class hint_cast_object
	{
	public:
		using hint_func_type = void (*)() noexcept;

		constexpr hint_cast_object() noexcept
			: hint(nullptr),
			  object(nullptr) {}

		template<typename T>
		constexpr hint_cast_object(T *_object) noexcept
			: hint(hint_traits<T>::hint),
			  object(_object) {}

		~hint_cast_object() = default;

		template<typename T>
		constexpr hint_cast_object & operator=(T *_object) noexcept
		{
			hint = hint_traits<T>::hint;
			object = _object;
			return *this;
		}

		template<typename U>
			requires std::is_pointer_v<U>
		constexpr U cast() noexcept
		{
			if(hint != hint_traits<std::remove_cv_t<std::remove_pointer_t<U>>>::hint)
				return nullptr;

			return static_cast<U *>(object);
		}

		constexpr void * release() noexcept
		{
			hint = nullptr;
			return object;
		}

		constexpr explicit operator bool() const noexcept
		{
			return hint;
		}

		constexpr bool has_object() const noexcept
		{
			return object != nullptr;
		}

		template<typename U>
		constexpr bool holds() const noexcept
		{
			return hint == hint_traits<U>::hint;
		}

		constexpr bool operator==(const hint_cast_object &obj) const noexcept = default;

		constexpr bool same(hint_func_type _hint) const noexcept
		{
			return hint == _hint;
		}

		constexpr bool same(const void *_object) const noexcept
		{
			return object == _object;
		}

	private:
		hint_func_type hint;
		void *object;
	};
};
