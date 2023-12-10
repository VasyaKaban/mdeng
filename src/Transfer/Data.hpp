/**
 * @file
 *
 * Represents the Data class for transfer operations
 */

#pragma once

#include <utility>

namespace FireLand
{
	/**
	 * @brief The Data class
	 *
	 * Used as pointer to data + deletetion function
	 */
	struct Data
	{
		std::byte *ptr;///<pointer to data
		void (*delete_function)(std::byte *);///<deletion function

		constexpr Data(std::byte *_ptr = nullptr, void (*_delete_function)(std::byte *) = nullptr)
			: ptr(_ptr), delete_function(_delete_function) {}

		~Data() = default;

		constexpr Data(const Data &) = default;
		constexpr Data(Data &&data) noexcept
			: ptr(data.ptr), delete_function(data.delete_function)
		{
			data.ptr = nullptr;
			data.delete_function = nullptr;
		}

		constexpr Data & operator=(const Data &) = default;
		constexpr Data & operator=(Data &&data) noexcept
		{
			Delete();
			ptr = data.ptr;
			delete_function = data.delete_function;
			data.ptr = nullptr;
			data.delete_function = nullptr;

			return *this;
		}

		void Delete() noexcept
		{
			if(delete_function)
			{
				delete_function(ptr);
				ptr = nullptr;
			}
		}

		constexpr explicit operator bool() const noexcept
		{
			return ptr;
		}
	};
};
