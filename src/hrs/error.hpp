#pragma once

#include <type_traits>
#include <concepts>
#include <cstdint>
#include <string_view>
#include "enum_meta.hpp"

namespace hrs
{
	union enum_value
	{
		char c;

		std::uint8_t u8t;
		std::int8_t s8t;

		std::uint16_t u16t;
		std::int16_t s16t;

		std::uint32_t u32t;
		std::int32_t s32t;

		std::uint64_t u64t;
		std::int64_t s64t;

		constexpr enum_value() noexcept : u8t(0) {}

		constexpr enum_value(char _c) noexcept : c(_c) {}
		constexpr enum_value(std::uint8_t _u8t) noexcept : u8t(_u8t) {}
		constexpr enum_value(std::int8_t _s8t) noexcept : s8t(_s8t) {}
		constexpr enum_value(std::uint16_t _u16t) noexcept : u16t(_u16t) {}
		constexpr enum_value(std::int16_t _s16t) noexcept : s16t(_s16t) {}
		constexpr enum_value(std::uint32_t _u32t) noexcept : u32t(_u32t) {}
		constexpr enum_value(std::int32_t _s32t) noexcept : s32t(_s32t) {}
		constexpr enum_value(std::uint64_t _u64t) noexcept : u64t(_u64t) {}
		constexpr enum_value(std::int64_t _s64t) noexcept : s64t(_s64t) {}

		template<std::integral I>
		constexpr auto get_same() const noexcept
		{
			if constexpr(std::same_as<I, char>)
				return c;
			else if constexpr(std::same_as<I, std::uint8_t>)
				return u8t;
			else if constexpr(std::same_as<I, std::int8_t>)
				return s8t;
			else if constexpr(std::same_as<I, std::uint16_t>)
				return u16t;
			else if constexpr(std::same_as<I, std::int16_t>)
				return s16t;
			else if constexpr(std::same_as<I, std::uint32_t>)
				return u32t;
			else if constexpr(std::same_as<I, std::int32_t>)
				return s32t;
			else if constexpr(std::same_as<I, std::uint64_t>)
				return u64t;
			else if constexpr(std::same_as<I, std::int64_t>)
				return s64t;
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr auto get() const noexcept
		{
			return static_cast<E>(get_same<std::underlying_type_t<E>>());
		}
	};

	template<typename E>
		requires std::is_enum_v<E>
	struct enum_error_traits
	{
		constexpr static void traits_hint() noexcept {};
		constexpr static std::string_view get_name(E value) noexcept
		{
			return enum_meta<E>::get_name(value);
		}
	};

	class error
	{
	public:

		using traits_hint_type = void (*)() noexcept;

		constexpr error() noexcept
			: traits_hint(nullptr) {}

		~error() = default;

		template<typename E>
			requires std::is_enum_v<E>
		constexpr error(E e) noexcept
			: value(static_cast<std::underlying_type_t<E>>(e)),
			  traits_hint(enum_error_traits<E>::traits_hint) {}

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
			return traits_hint == enum_error_traits<E>::traits_hint;
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr E revive() const noexcept
		{
			return (holds<E>() ? value.get<E>() : E{});
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr std::string_view get_name() const noexcept
		{
			return (traits_hint == enum_error_traits<E>::traits_hint ?
						enum_error_traits<E>::get_name(value.get<E>()) :
						"");
		}

		constexpr enum_value get_raw_value() const noexcept
		{
			return value;
		}

		template<typename E>
			requires std::is_enum_v<E>
		constexpr bool operator==(E e) const noexcept
		{
			return (traits_hint == enum_error_traits<E>::traits_hint ? value.get<E>() == e : false);
		}

	private:

		template<typename E, typename ...Enums, typename F>
		constexpr bool visit_one(F &&f) const noexcept
		{
			if(enum_error_traits<E>::traits_hint == traits_hint)
			{
				if constexpr(std::invocable<F, E>)
					std::forward<F>(f)(value.get<E>());

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
				std::invocable<F, enum_value> ||
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
					if constexpr(std::invocable<F, enum_value>)
						std::forward<F>(f)(value);
				}
				else
				{
					bool visit_res = visit_one<Enums...>(std::forward<F>(f));
					if constexpr(std::invocable<F, enum_value>)
					{
						if(!visit_res)
							std::forward<F>(f)(value);
					}
				}
			}
		}
	private:
		enum_value value;
		traits_hint_type traits_hint;
	};
};
