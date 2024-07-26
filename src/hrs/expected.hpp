#pragma once

#include <concepts>
#include <utility>

namespace hrs
{
	struct unexpected_t {};

	constexpr inline unexpected_t unexpected;

	template<typename T, typename E>
	class expected
	{
	public:

		using value_type = T;
		using error_type = E;

		constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
			requires std::is_default_constructible_v<T>
			: is_error(false) {}

		template<typename U = T>
			requires std::constructible_from<T, U>
		constexpr expected(U &&val) noexcept(std::is_nothrow_constructible_v<T, U>)
			: data(std::forward<U>(val)),
			  is_error(false) {}

		template<typename U = E>
			requires std::constructible_from<E, U> && (!std::constructible_from<T, U>)
		constexpr expected(U &&err) noexcept(std::is_nothrow_constructible_v<E, U>)
			: data(std::forward<U>(err)),
			  is_error(false) {}

		template<typename U = E>
			requires std::assignable_from<E &, U>
		constexpr expected(U &&err, unexpected_t _) noexcept(std::is_nothrow_assignable_v<E &, U>)
		{
			is_error = true;
			data.error = std::forward<U>(err);
		}

		constexpr ~expected()
		{
			if(is_error)
				data.error.~E();
			else
				data.value.~T();
		}

		constexpr expected(const expected &ex) noexcept(std::is_nothrow_copy_assignable_v<T> &&
														std::is_nothrow_copy_assignable_v<E>)
		{
			if(ex.is_error)
				*this = ex.data.error;
			else
				*this = ex.data.value;
		}

		constexpr expected(expected &&ex) noexcept(std::is_nothrow_move_assignable_v<T> &&
												   std::is_nothrow_move_assignable_v<E>)
		{
			if(ex.is_error)
				*this = std::move(ex).data.error;
			else
				*this = std::move(ex).data.value;
		}

		constexpr expected & operator=(const expected &ex) noexcept(std::is_nothrow_copy_assignable_v<T> &&
																	std::is_nothrow_copy_assignable_v<E>)
		{
			this->~expected();

			if(ex.is_error)
				*this = ex.data.error;
			else
				*this = ex.data.value;

			return *this;
		}

		constexpr expected & operator=(expected &&ex) noexcept(std::is_nothrow_move_assignable_v<T> &&
															   std::is_nothrow_move_assignable_v<E>)
		{
			this->~expected();

			if(ex.is_error)
				*this = std::move(ex).data.error;
			else
				*this = std::move(ex).data.value;

			return *this;
		}

		template<typename U = T>
			requires std::assignable_from<T &, U>
		constexpr expected & operator=(U &&value) noexcept(std::is_nothrow_assignable_v<T &, U>)
		{
			this->~expected();
			is_error = false;
			data.value = std::forward<U>(value);
			return *this;
		}

		template<typename U = E>
			requires std::assignable_from<E &, U> && (!std::assignable_from<T &, U>)
		constexpr expected & operator=(U &&error) noexcept(std::is_nothrow_assignable_v<E &, U>)
		{
			this->~expected();
			is_error = true;
			data.error = std::forward<U>(error);
			return *this;
		}

		constexpr explicit operator bool() const noexcept
		{
			return !is_error;
		}

		constexpr E & error() & noexcept
		{
			return data.error;
		}

		constexpr const E & error() const & noexcept
		{
			return data.error;
		}

		constexpr E && error() && noexcept
		{
			return std::move(data.error);
		}

		constexpr const E && error() const && noexcept
		{
			return std::move(data.error);
		}

		constexpr T & value() & noexcept
		{
			return data.value;
		}

		constexpr const T & value() const & noexcept
		{
			return data.value;
		}

		constexpr T && value() && noexcept
		{
			return std::move(data.value);
		}

		constexpr const T && value() const && noexcept
		{
			return std::move(data.value);
		}

		constexpr bool has_value() const noexcept
		{
			return !is_error;
		}

		constexpr T * operator->() noexcept
		{
			return &value();
		}

		constexpr const T * operator->() const noexcept
		{
			return &value();
		}

		constexpr T & operator*() & noexcept
		{
			return value();
		}

		constexpr const T & operator*() const & noexcept
		{
			return value();
		}

		constexpr T && operator*() && noexcept
		{
			return value();
		}

		constexpr const T && operator*() const && noexcept
		{
			return value();
		}

		template<typename U>
		constexpr T value_or(U &&other) const & noexcept(std::is_nothrow_convertible_v<U, T>)
		{
			if(has_value())
				return **this;

			return std::forward<U>(other);
		}

		template<typename U>
		constexpr T value_or(U &&other) const && noexcept(std::is_nothrow_convertible_v<U, T>)
		{
			if(has_value())
				return **this;

			return std::forward<U>(other);
		}

		template<typename F>
		constexpr hrs::expected<T, E> and_then(F &&func) &
		{
			if(has_value())
				return std::forward<F>(func)(**this);
			else
				return error();
		}

		template<typename F>
		constexpr hrs::expected<T, E> and_then(F &&func) const &
		{
			if(has_value())
				return std::forward<F>(func)(**this);
			else
				return error();
		}

		template<typename F>
		constexpr hrs::expected<T, E> and_then(F &&func) &&
		{
			if(has_value())
				return std::forward<F>(func)(std::move(**this));
			else
				return error();
		}

		template<typename F>
		constexpr hrs::expected<T, E> and_then(F &&func) const &&
		{
			if(has_value())
				return std::forward<F>(func)(std::move(**this));
			else
				return error();
		}

		template<typename F>
		constexpr hrs::expected<T, E> or_else(F &&func) &
		{
			if(!has_value())
				return std::forward<F>(func)(error());
			else
				return value();
		}

		template<typename F>
		constexpr hrs::expected<T, E> or_else(F &&func) const &
		{
			if(!has_value())
				return std::forward<F>(func)(error());
			else
				return value();
		}

		template<typename F>
		constexpr hrs::expected<T, E> or_else(F &&func) &&
		{
			if(!has_value())
				return std::forward<F>(func)(std::move(error()));
			else
				return value();
		}

		template<typename F>
		constexpr hrs::expected<T, E> or_else(F &&func) const &&
		{
			if(!has_value())
				return std::forward<F>(func)(std::move(error()));
			else
				return value();
		}


	private:
		union expected_data
		{
			T value;
			E error;

			constexpr expected_data() noexcept(std::is_nothrow_default_constructible_v<T>)
				requires std::is_default_constructible_v<T>
				: value{} {}

			constexpr ~expected_data() {}

			template<typename U = T>
				requires std::constructible_from<T, U>
			constexpr expected_data(U &&val) noexcept(std::is_nothrow_constructible_v<T, U>)
				: value(std::forward<U>(val)) {}

			template<typename U = E>
				requires std::constructible_from<E, U> && (!std::constructible_from<T, U>)
			constexpr expected_data(U &&err) noexcept(std::is_nothrow_constructible_v<E, U>)
				: error(std::forward<U>(err)) {}

		} data;
		bool is_error;
	};
}
