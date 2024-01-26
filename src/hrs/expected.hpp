/**
 * @file
 *
 * Represents expected and unexpected types
 */

#pragma once

#include <utility>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <cstring>
#include <version>

#ifdef __cpp_lib_start_lifetime_as
#include <memory>
#endif

namespace hrs
{	
	/**
	 * @brief convert_to_type_ptr
	 * @tparam E type to cast
	 * @param ptr pointer to data
	 * @return ptr casted to E *
	 *
	 * Uses std::start_lifetime_as if __cpp_lib_start_lifetime_as is supported.
	 * Otherwise uses reinterpret_cast.
	 */
	template<typename E>
	constexpr E * convert_to_type_ptr(void *ptr)
	{
	#ifdef __cpp_lib_start_lifetime_as
		return std::start_lifetime_as<E>(ptr);
	#else
		return reinterpret_cast<E *>(ptr);
	#endif
	}

	/**
	 * @brief The union_size_alignment class
	 */
	struct union_size_alignment
	{
		size_t size;///<the size of union block
		size_t alignment;///<the alignment of union block
	};

	/**
	 * @brief common_size_align
	 * @tparam T the first type
	 * @tparam Args other types
	 * @return the union_size_alignment object that containts common size and max alignment for passed types
	 */
	template<typename T, typename ...Args>
	consteval union_size_alignment common_size_align()
	{
		size_t max_size = std::max({sizeof(T), sizeof(Args)...});
		size_t max_align = std::max({alignof(T), alignof(Args)...});

		if(max_size > max_align)
		{
			if(max_size % max_align != 0)
				max_size = (max_size / max_align + 1) * max_align;
		}

		return {max_size, max_align};
	}

	/**
	 * @brief The unexpected class
	 * @tparam E error type
	 *
	 * Used for expected class when it contains same error and value types
	 */
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

	/**
	 * @brief The expected class
	 * @tparam T value type
	 * @tparam E error type
	 *
	 * At one moment can include only value or error
	 */
	template<typename T, typename E>
	class expected
	{
	public:

		using value_type = T;
		using error_type = E;

		constexpr inline static size_t data_size = common_size_align<T, E>().size;
		constexpr inline static size_t data_alignment = common_size_align<T, E>().alignment;

		constexpr expected() noexcept(std::is_nothrow_default_constructible_v<T>)
			requires std::is_default_constructible_v<T>
		{
			is_error = false;
			new(union_block) T{};
		}

		constexpr expected(const T &val) noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires std::is_copy_constructible_v<T>
		{
			is_error = false;
			new(union_block) T{val};
		}

		constexpr expected(T &&val) noexcept(std::is_nothrow_move_constructible_v<T>)
			requires std::is_move_constructible_v<T>
		{
			is_error = false;
			new(union_block) T{std::move(val)};
		}

		constexpr expected(const E &err) noexcept(std::is_nothrow_copy_constructible_v<T>)
			requires (!std::is_same_v<T, E>) && std::is_copy_constructible_v<E>
		{
			is_error = true;
			new(union_block) E{err};
		}

		constexpr expected(E &&err) noexcept(std::is_nothrow_move_constructible_v<T>)
			requires (!std::is_same_v<T, E>) && std::is_move_constructible_v<E>
		{
			is_error = true;
			new(union_block) E{std::move(err)};
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
				convert_to_type_ptr<E>(union_block)->~E();
			else
				convert_to_type_ptr<T>(union_block)->~T();
		}

		constexpr expected(const expected &ex) noexcept(std::is_nothrow_copy_constructible_v<T> &&
														std::is_nothrow_copy_constructible_v<E>)
		{
			if(ex.is_error)
				*this = ex.error();
			else
				*this = ex.value();

			is_error = ex.is_error;
		}

		constexpr expected(expected &&ex) noexcept(std::is_nothrow_move_constructible_v<T> &&
												   std::is_nothrow_move_constructible_v<E>)
		{
			if(ex.is_error)
				*this = std::move(ex.error());
			else
				*this = std::move(ex.value());
			is_error = ex.is_error;
			ex.is_error = false;
			ex = T{};
		}

		constexpr expected & operator=(expected &&ex) noexcept(std::is_nothrow_move_constructible_v<T> &&
															   std::is_nothrow_move_constructible_v<E>)
		{
			this->~expected();
			if(ex.is_error)
				*this = std::move(ex.error());
			else
				*this = std::move(ex.value());
			is_error = ex.is_error;
			ex.is_error = false;
			ex = T{};
			return *this;
		}

		constexpr expected & operator=(const expected &ex) noexcept(std::is_nothrow_copy_constructible_v<T> &&
																	   std::is_nothrow_copy_constructible_v<E>)
		{
			this->~expected();
			if(ex.is_error)
				*this = ex.error();
			else
				*this = ex.value();

			is_error = ex.is_error;

			return *this;
		}

		constexpr expected & operator=(const T &value) noexcept(std::is_nothrow_copy_assignable_v<T>)
		{
			this->~expected();
			is_error = false;
			new(union_block) T{value};
			return *this;
		}

		constexpr expected & operator=(T &&value) noexcept(std::is_nothrow_move_assignable_v<T>)
		{
			this->~expected();
			is_error = false;
			new(union_block) T{std::move(value)};
			return *this;
		}

		constexpr expected & operator=(const E &error) noexcept(std::is_nothrow_copy_assignable_v<E>)
			requires (!std::same_as<T, E>)
		{
			this->~expected();
			is_error = true;
			new(union_block) E{error};
			return *this;
		}

		constexpr expected & operator=(E &&error) noexcept(std::is_nothrow_move_assignable_v<E>)
			requires (!std::same_as<T, E>)
		{
			this->~expected();
			is_error = true;
			new(union_block) E{std::move(error)};
			return *this;
		}

		/**
		 * @brief operator bool
		 *
		 * returns true if contains value
		 */
		constexpr explicit operator bool() const noexcept
		{
			return !is_error;
		}

		/**
		 * @brief error
		 * @return reference to error
		 *
		 * Explicitly casts inner data to error type without checks
		 */
		constexpr E & error() noexcept
		{
			return *convert_to_type_ptr<E>(union_block);
		}

		/**
		 * @brief error
		 * @return const reference to error
		 *
		 * Explicitly casts inner data to error type without checks
		 */
		constexpr const E & error() const noexcept
		{
			return *convert_to_type_ptr<E>(union_block);
		}

		/**
		 * @brief value
		 * @return reference to value
		 *
		 * Explicitly casts inner data to value type without checks
		 */
		constexpr T & value() noexcept
		{
			return *convert_to_type_ptr<T>(union_block);
		}

		/**
		 * @brief value
		 * @return const reference to value
		 *
		 * Explicitly casts inner data to value type without checks
		 */
		constexpr const T & value() const noexcept
		{
			return *convert_to_type_ptr<T>(union_block);
		}

		/**
		 * @brief has_value
		 * @return true if contains value, otherwise false
		 *
		 * Checks whether inner block contains value
		 */
		constexpr bool has_value() const noexcept
		{
			return !is_error;
		}

		/**
		 * @brief operator ->
		 * @return pointer to value type
		 *
		 * Explicitly casts inner data to pointer to value type
		 */
		constexpr T * operator->() noexcept
		{
			return convert_to_type_ptr<T>(union_block);
		}

		/**
		 * @brief operator ->
		 * @return pointer to value type
		 *
		 * Explicitly casts inner data to pointer to const value type
		 */
		constexpr const T * operator->() const noexcept
		{
			return convert_to_type_ptr<T>(union_block);
		}

		/**
		 * @brief operator *
		 * @return reference to value type
		 *
		 * Explicitly casts inner data to reference to value type
		 */
		constexpr T & operator*() noexcept
		{
			return *convert_to_type_ptr<T>(union_block);
		}

		/**
		 * @brief operator *
		 * @return reference to value type
		 *
		 * Explicitly casts inner data to const reference to value type
		 */
		constexpr const T & operator*() const noexcept
		{
			return *convert_to_type_ptr<T>(union_block);
		}

	private:
		///a block of data that contains a value or error
		alignas(data_alignment) uint8_t union_block[data_size];
		///a flag that is used for checking whether a data block contains an error or value
		bool is_error;
	};
}
