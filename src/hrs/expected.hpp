#pragma once

#include "mem_req.hpp"
#include "instantiation.hpp"

namespace hrs
{
	template<typename U>
	constexpr U * start_lifetime_from_bytes(std::byte *ptr) noexcept
	{
		return std::launder(reinterpret_cast<U *>(ptr));
	}

	template<typename U>
	constexpr const U * start_lifetime_from_bytes(const std::byte *ptr) noexcept
	{
		return std::launder(reinterpret_cast<const U *>(ptr));
	}

	template<typename E>
	struct unexpected
	{
		E error;

		unexpected() requires std::is_default_constructible_v<E> = default;
		~unexpected() = default;
		constexpr unexpected(const unexpected &un) noexcept(std::is_nothrow_copy_constructible_v<E>)
			: error(un.error) {}
		constexpr unexpected(unexpected &&un) noexcept(std::is_nothrow_move_constructible_v<E>)
			: error(std::move(un.error)) {}

		constexpr unexpected & operator=(const E &err) noexcept(std::is_nothrow_copy_assignable_v<E>)
		{
			error = err;
			return *this;
		}

		constexpr unexpected & operator=(E &&err) noexcept(std::is_nothrow_move_assignable_v<E>)
		{
			error = std::move(err);
			return *this;
		}
	};

	template<typename T, typename E>
	class expected
	{
	public:

		using value_type = T;
		using error_type = E;

		constexpr inline static size_t DATA_SIZE = union_size_alignment<T, E>().size;
		constexpr inline static size_t DATA_ALIGNMENT = union_size_alignment<T, E>().alignment;

		constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
			requires std::is_default_constructible_v<T>
		{
			is_error = false;
			new(union_block) T{};
		}

		template<typename U = T>
			requires std::constructible_from<T, U>
		constexpr expected(U &&val) noexcept(std::is_nothrow_constructible_v<T, U>)
		{
			is_error = false;
			new(union_block) T(std::forward<U>(val));
		}

		template<typename U = E>
			requires std::constructible_from<E, U> && (!std::constructible_from<T, U>)
		constexpr expected(U &&err) noexcept(std::is_nothrow_constructible_v<E, U>)
		{
			is_error = true;
			new(union_block) E(std::forward<U>(err));
		}

		constexpr expected(const unexpected<E> &unex) noexcept(std::is_nothrow_copy_constructible_v<E>)
			requires std::is_copy_constructible_v<E>
		{
			is_error = true;
			new(union_block) E{unex.error};
		}

		constexpr expected(unexpected<E> &&unex) noexcept(std::is_nothrow_move_constructible_v<E>)
			requires std::is_move_constructible_v<E>
		{
			is_error = true;
			new(union_block) E{std::move(unex.error)};
		}

		constexpr ~expected()
		{
			if(is_error)
				start_lifetime_from_bytes<E>(union_block)->~E();
			else
				start_lifetime_from_bytes<T>(union_block)->~T();
		}

		constexpr expected(const expected &ex) noexcept(std::is_nothrow_copy_constructible_v<T> &&
														std::is_nothrow_copy_constructible_v<E>)
		{
			*this = (ex.is_error ? ex.error() : ex.value());
			is_error = ex.is_error;
		}

		constexpr expected(expected &&ex) noexcept(std::is_nothrow_move_constructible_v<T> &&
												   std::is_nothrow_move_constructible_v<E>)
		{
			*this = (ex.is_error ? std::move(ex.error()) : std::move(ex.value()));
			ex = expected{};
		}

		constexpr expected & operator=(const expected &ex) noexcept(std::is_nothrow_copy_constructible_v<T> &&
																	std::is_nothrow_copy_constructible_v<E>)
		{
			this->~expected();

			*this = (ex.is_error ? ex.error() : ex.value());
			is_error = ex.is_error;

			return *this;
		}

		constexpr expected & operator=(expected &&ex) noexcept(std::is_nothrow_move_constructible_v<T> &&
															   std::is_nothrow_move_constructible_v<E>)
		{
			this->~expected();

			*this = (ex.is_error ? std::move(ex.error()) : std::move(ex.value()));
			ex = expected{};

			return *this;
		}

		template<typename U = T>
			requires std::constructible_from<T, U>
		constexpr expected & operator=(U &&value) noexcept(std::is_nothrow_constructible_v<T, U>)
		{
			this->~expected();
			is_error = false;
			new(union_block) T(std::forward<U>(value));
			return *this;
		}

		template<typename U = E>
			requires std::constructible_from<E, U> && (!std::constructible_from<T, U>)
		constexpr expected & operator=(U &&error) noexcept(std::is_nothrow_constructible_v<E, U>)
		{
			this->~expected();
			is_error = true;
			new(union_block) E(std::forward<U>(error));
			return *this;
		}

		constexpr explicit operator bool() const noexcept
		{
			return !is_error;
		}

		constexpr E & error() & noexcept
		{
			return *start_lifetime_from_bytes<E>(union_block);
		}

		constexpr const E & error() const & noexcept
		{
			return *start_lifetime_from_bytes<E>(union_block);
		}

		constexpr E && error() && noexcept
		{
			return std::move(*start_lifetime_from_bytes<E>(union_block));
		}

		constexpr const E && error() const && noexcept
		{
			return std::move(*start_lifetime_from_bytes<E>(union_block));
		}

		constexpr T & value() & noexcept
		{
			return *start_lifetime_from_bytes<T>(union_block);
		}

		constexpr const T & value() const & noexcept
		{
			return *start_lifetime_from_bytes<T>(union_block);
		}

		constexpr T && value() && noexcept
		{
			return std::move(*start_lifetime_from_bytes<T>(union_block));
		}

		constexpr const T && value() const && noexcept
		{
			return std::move(*start_lifetime_from_bytes<T>(union_block));
		}

		constexpr bool has_value() const noexcept
		{
			return !is_error;
		}

		constexpr T * operator->() noexcept
		{
			return start_lifetime_from_bytes<T>(union_block);
		}

		constexpr const T * operator->() const noexcept
		{
			return start_lifetime_from_bytes<T>(union_block);
		}

		constexpr T & operator*() & noexcept
		{
			return *start_lifetime_from_bytes<T>(union_block);
		}

		constexpr const T & operator*() const & noexcept
		{
			return *start_lifetime_from_bytes<T>(union_block);
		}

		constexpr T && operator*() && noexcept
		{
			return std::move(*start_lifetime_from_bytes<T>(union_block));
		}

		constexpr const T && operator*() const && noexcept
		{
			return std::move(*start_lifetime_from_bytes<T>(union_block));
		}

	private:
		alignas(DATA_ALIGNMENT) std::byte union_block[DATA_SIZE];
		bool is_error;
	};
}





