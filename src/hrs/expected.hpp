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
			requires std::assignable_from<T &, U>
		constexpr expected(U &&val) noexcept(std::is_nothrow_assignable_v<T &, U>)
		{
			is_error = false;
			data.value = std::forward<U>(val);
		}

		template<typename U = E>
			requires std::assignable_from<E &, U> && (!std::assignable_from<T &, U>)
		constexpr expected(U &&err) noexcept(std::is_nothrow_assignable_v<E &, U>)
		{
			is_error = true;
			data.error = std::forward<U>(err);
		}

		template<typename U = E>
			requires std::assignable_from<E &, U>
		constexpr expected(unexpected_t _, U &&err) noexcept(std::is_nothrow_assignable_v<E &, U>)
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
				: value() {}

			constexpr ~expected_data() {}
		} data;
		bool is_error;
	};
}

namespace hrs
{
	template<typename T>
		requires std::is_move_assignable_v<T> || std::is_copy_assignable_v<T>
	void move_if_possible(T &out, T &val) noexcept(std::is_nothrow_move_assignable_v<T> ||
												   std::is_nothrow_copy_assignable_v<T>)
	{
		if constexpr(std::is_move_assignable_v<T>)
			out = std::move(val);
		else
			out = val;
	}
};

#define HRS_PROPAGATE_EXP(val, op) \
	static_assert(std::is_default_constructible_v<std::remove_reference_t<decltype(*op)>>); \
	std::remove_reference_t<decltype(*op)> val; \
	{ \
		auto exp = op; \
		if(!exp) \
			return exp.error(); \
		else \
			hrs::move_if_possible(val, *exp); \
	}

#include <string>

auto foo(bool b) -> hrs::expected<std::string, int>
{
	if(b)
		return 1;

	return "hello!";
}

auto bar() -> hrs::expected<std::string, float>
{
	HRS_PROPAGATE_EXP(val_get, foo(true))
	return val_get;
}

auto first_foo(std::string val) -> hrs::expected<std::string, int>
{
	std::string str(std::move(val));
	return str.append("amogus");
}

auto second_foo(std::string val) -> hrs::expected<std::string, int>
{
	if(val.empty())
		return 1;

	std::string str(std::move(val));

	//return {hrs::unexpected, 1};

	return str.append("abobus");
}

auto func()
{
	auto res = foo(true)
				   .and_then
			   ([](std::string val)
				{
					std::string str(std::move(val));
					return str.append("amogus");
				})
				   .and_then
			   ([](std::string val) -> hrs::expected<std::string, int>
				{
					if(val.empty())
						return 1;

					std::string str(std::move(val));
					return str.append("abobus");
				})
				   .or_else
			   ([](int err)
				{
					return "Hello!";
				})
				   .value_or("");
}
