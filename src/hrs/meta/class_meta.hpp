#pragma once

#include <tuple>
#include "../variadic.hpp"
#include "../static_string/static_string.hpp"
#include "meta_attributes.hpp"
#include "split_namespace.hpp"

namespace hrs
{
	namespace detail
	{
		template<typename>
		struct member_class_type {};

		template<typename C, typename T>
		struct member_class_type<T C::*>
		{
			using field_type = T;
			using class_type = C;
		};

		template<typename C,
				  hrs::static_string _name,
				  hrs::non_type_instantiation<meta_attributes> _attributes>
		struct class_meta_base : _attributes
		{
			constexpr static auto name = _name;
			using parents = hrs::variadic<>;
			using refl_class = C;
			using member_fields = hrs::variadic<>;
			using static_fields = hrs::variadic<>;
			using using_fields = hrs::variadic<>;
			constexpr static auto namespaces = std::tuple{};
		};
	};

	template<auto _ptr, hrs::static_string _name, auto ..._attrs>
	struct member_field : public meta_attributes<_attrs...>
	{
		using type = detail::member_class_type<decltype(_ptr)>::field_type;
		constexpr static auto name = _name;
		constexpr static auto ptr = _ptr;
		using class_type = typename detail::member_class_type<decltype(_ptr)>::class_type;

		template<typename C>
			requires
				(std::is_class_v<class_type> && std::is_base_of_v<class_type, std::remove_cvref_t<C>>) ||
				(std::is_union_v<class_type>)
		constexpr static auto & get(C &&obj) noexcept
		{
			return std::forward<C>(obj).*ptr;
		}
	};

	template<auto _ptr, hrs::static_string _name, typename C, auto ..._attrs>
	struct static_field : public meta_attributes<_attrs...>
	{
		using type = std::remove_pointer_t<decltype(_ptr)>;
		constexpr static auto name = _name;
		constexpr static auto ptr = _ptr;
		using class_type = C;

		constexpr static auto & get() noexcept
		{
			return *ptr;
		}
	};

	template<typename T, hrs::static_string _name, typename C, auto ..._attrs>
	struct using_field : public meta_attributes<_attrs...>
	{
		using type = T;
		constexpr static auto name = _name;
		using class_type = C;
	};

	template<typename C, typename ...Classes>
	consteval bool is_derived_from(hrs::variadic<Classes...>) noexcept
	{
		return (std::is_base_of_v<Classes, C> && ...);
	}

	template<typename C>
	struct class_meta : detail::class_meta_base<C, "", meta_attributes<>> {};
};
