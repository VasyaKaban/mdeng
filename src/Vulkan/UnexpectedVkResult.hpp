#pragma once

#include "../hrs/unexpected_result.hpp"
#include "VulkanInclude.hpp"

namespace FireLand
{
	class UnexpectedVkResult : public hrs::unexpected_result_interface
	{
	public:
		UnexpectedVkResult(vk::Result _result,
						   const std::source_location &_location = std::source_location::current()) noexcept
			: result(_result), location(_location) {}

		UnexpectedVkResult(const UnexpectedVkResult &)  = default;
		UnexpectedVkResult(UnexpectedVkResult &&) = default;
		UnexpectedVkResult & operator=(const UnexpectedVkResult &) = default;
		UnexpectedVkResult & operator=(UnexpectedVkResult &&) = default;

		virtual ~UnexpectedVkResult() = default;

		virtual const std::source_location & GetSourceLocation() const noexcept
		{
			return location;
		}

		virtual std::string GetErrorMessage()
		{
			return vk::to_string(result);
		}

		constexpr vk::Result GetResult() const noexcept
		{
			return result;
		}

	private:
		vk::Result result;
		std::source_location location;
	};
};
