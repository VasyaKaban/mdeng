#pragma once

#include <variant>
#include "debug.hpp"
#include "instantiation.hpp"

namespace hrs
{
	template<typename T, typename ...Args>
	concept is_one_of = (std::same_as<T, Args> || ...);

	template<typename ...Args>
		requires (std::is_enum_v<Args> && ...)
	class error
	{
	public:
		constexpr error() = default;
		~error() = default;

		template<is_one_of<Args...> T>
		constexpr error(T value) noexcept : var(value) {}

		template<typename ...AArgs>
			requires (sizeof...(AArgs) <= sizeof...(Args)) && (is_one_of<AArgs, Args...> && ...)
		constexpr error(const error<AArgs...> &err) noexcept
		{
			err.visit([this](const auto val)
			{
				var = val;
			});
		}

		template<typename ...AArgs>
			requires (sizeof...(AArgs) <= sizeof...(Args)) && (is_one_of<AArgs, Args...> && ...)
		constexpr error & operator=(const error &err) noexcept
		{
			err.visit([this](const auto val)
			{
				 var = val;
			});

			return *this;
		}

		template<is_one_of<Args...> T>
		constexpr bool keeps() const noexcept
		{
			return std::visit([]<typename V>(const V val)
			{
				return std::same_as<T, V>;
			}, var);
		}

		template<is_one_of<Args...> T>
		constexpr T get() const noexcept
		{
			hrs::assert_true_debug(keeps<T>(), "Error doesn't keep the desired type now!");
			return std::get<T>(var);
		}

		template<typename F>
			requires (std::invocable<F, Args> && ...)
		constexpr auto visit(F &&f) const noexcept((std::is_nothrow_invocable_v<F, Args> && ...))
		{
			return std::visit(std::forward<F>(f), var);
		}

		template<typename ...AArgs>
			requires (sizeof...(AArgs) <= sizeof...(Args)) && (is_one_of<AArgs, Args...> && ...)
		constexpr bool operator==(const error<AArgs...> &err) const noexcept
		{
			return err.visit([this]<typename T1>(const T1 val1)
			{
				return visit([val1]<typename T2>(const T2 val2)
				{
					if constexpr(std::same_as<T1, T2>)
						return val1 == val2;
					else
						return false;
				});
			});
		}

	private:
		std::variant<Args...> var;
	};
};
