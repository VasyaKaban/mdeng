#pragma once

#include <memory>
#include <source_location>
#include "non_creatable.hpp"

namespace hrs
{
	class unexpected_result_interface
	{
	public:
		virtual ~unexpected_result_interface() {};
		virtual const std::source_location & GetSourceLocation() const noexcept = 0;
		virtual std::string GetErrorMessage() const = 0;
	};

	class unexpected_result : public hrs::non_copyable
	{
	public:

		unexpected_result() = default;

		template<std::derived_from<unexpected_result_interface> E>
			requires
				std::copy_constructible<std::remove_reference_t<E>> ||
				std::move_constructible<std::remove_reference_t<E>>
		unexpected_result(E &&error_object)
			: error(new std::remove_cvref_t<E>(std::forward<E>(error_object))) {}

		~unexpected_result() = default;

		unexpected_result(const unexpected_result &unexpected) noexcept
			: error(unexpected.error) {}

		unexpected_result(unexpected_result &&unexpected) noexcept
			: error(std::move(unexpected.error)) {}

		unexpected_result & operator=(const unexpected_result &unexpected) noexcept
		{
			error.reset();
			error = unexpected.error;
			return *this;
		}

		unexpected_result & operator=(unexpected_result &&unexpected) noexcept
		{
			error.reset();
			error = std::move(unexpected.error);
			return *this;
		}

		template<std::derived_from<unexpected_result_interface> E>
		unexpected_result & operator=(E &&error_object)
		{
			error.reset(new std::remove_cvref_t<E>(std::forward<E>(error_object)));
			return *this;
		}

		explicit operator bool() const noexcept
		{
			return static_cast<bool>(error);
		}

		unexpected_result_interface * GetValue() const noexcept
		{
			return error.get();
		}

		void Drop() noexcept
		{
			error.reset();
		}

	private:
		std::shared_ptr<unexpected_result_interface> error;
	};
};
