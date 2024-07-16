#pragma once

#include <type_traits>
#include <concepts>
#include <string_view>
#include "meta/enum_meta.hpp"

namespace hrs
{
	using error_enum_value = unsigned long long int;

	namespace detail
	{
		template<typename E>
			requires std::is_enum_v<E>
		constexpr error_enum_value enum_to_value(E e) noexcept
		{
			using enum_cast_t = std::conditional_t<std::is_signed_v<std::underlying_type_t<E>>,
												   signed long long int,
												   unsigned long long int>;

			return std::bit_cast<error_enum_value>(static_cast<enum_cast_t>(e));
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr E value_to_enum(error_enum_value value) noexcept
		{
			using enum_cast_t = std::conditional_t<std::is_signed_v<std::underlying_type_t<E>>,
												   signed long long int,
												   unsigned long long int>;

			return static_cast<E>(std::bit_cast<enum_cast_t>(value));
		}
	};

	template<typename E>
		requires std::is_enum_v<E>
	struct enum_error_traits
	{
		constexpr static std::string_view get_name(error_enum_value value) noexcept
		{
			return enum_meta<E>::get_name(value_to_enum<E>(value));
		}
	};

	class error
	{
	public:

		using traits_hint_type = std::string_view (*)(error_enum_value) noexcept;

		constexpr error() noexcept
			: value(0),
			  traits_hint(nullptr) {}

		~error() = default;

		template<typename E>
			requires std::is_enum_v<E>
		constexpr error(E e) noexcept
			: value(enum_to_value(e)),
			  traits_hint(enum_error_traits<E>::get_name) {}

		constexpr error(const error &) = default;
		constexpr error & operator=(const error &) = default;

		constexpr explicit operator bool() const noexcept
		{
			return traits_hint;
		}

		constexpr bool is_empty() const noexcept
		{
			return traits_hint == nullptr;
		}

		constexpr void clear() noexcept
		{
			traits_hint = nullptr;
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr bool holds() const noexcept
		{
			return traits_hint == enum_error_traits<E>::get_name;
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr E revive() const noexcept
		{
			return (holds<E>() ? detail::value_to_enum<E>(value) : E{});
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr std::string_view get_name() const noexcept
		{
			return (traits_hint == enum_error_traits<E>::get_name ?
						enum_error_traits<E>::get_name(detail::value_to_enum<E>(value)) :
						"");
		}

		constexpr error_enum_value get_raw_value() const noexcept
		{
			return value;
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr bool operator==(E e) const noexcept
		{
			return (traits_hint == enum_error_traits<E>::get_name ? detail::value_to_enum<E>(value) == e : false);
		}

	private:

		template<typename E, typename ...Enums, typename F>
		constexpr bool visit_one(F &&f) const noexcept
		{
			if(enum_error_traits<E>::get_name == traits_hint)
			{
				if constexpr(std::invocable<F, E>)
					std::forward<F>(f)(detail::value_to_enum<E>(value));

				return true;
			}

			if constexpr(sizeof...(Enums) != 0)
				return visit_one<Enums...>(std::forward<F>(f));
			else
				return false;
		}

	public:

		template<typename ...Enums, typename F>
			requires
				(std::is_enum_v<Enums> && ...) &&
				((std::invocable<F, Enums> && ...) ||
				std::invocable<F, error_enum_value> ||
				std::invocable<F>)
		constexpr void visit(F &&f) const noexcept
		{
			//if traits_hint == nullptr -> F::operator(void) <-> finally()
			//if traits_hint != Enums -> F::operator(enum_value) <-> catch(...)
			//if traits_hint == Enums[traits_hint] -> F::operator(value.get<Enums>) <-> catch(Enums)
			if(traits_hint == nullptr)
			{
				if constexpr(std::invocable<F>)
					std::forward<F>(f)();
			}
			else
			{
				if constexpr(sizeof...(Enums) == 0)
				{
					if constexpr(std::invocable<F, error_enum_value>)
						std::forward<F>(f)(value);
				}
				else
				{
					bool visit_res = visit_one<Enums...>(std::forward<F>(f));
					if constexpr(std::invocable<F, error_enum_value>)
					{
						if(!visit_res)
							std::forward<F>(f)(value);
					}
				}
			}
		}
	private:
		error_enum_value value;
		traits_hint_type traits_hint;
	};
};
