#pragma once

#include "../hrs/unexpected_result.hpp"
#include "AllocatorTypes.hpp"

namespace FireLand
{
	class UnexpectedAllocationResult : public hrs::unexpected_result_interface
	{
	public:
		UnexpectedAllocationResult(AllocationResult _result,
								   const std::source_location &_location = std::source_location::current()) noexcept
			: result(_result), location(_location) {}

		UnexpectedAllocationResult(const UnexpectedAllocationResult &)  = default;
		UnexpectedAllocationResult(UnexpectedAllocationResult &&) = default;
		UnexpectedAllocationResult & operator=(const UnexpectedAllocationResult &) = default;
		UnexpectedAllocationResult & operator=(UnexpectedAllocationResult &&) = default;

		virtual ~UnexpectedAllocationResult() = default;

		virtual const std::source_location & GetSourceLocation() const noexcept
		{
			return location;
		}

		virtual std::string GetErrorMessage()
		{
			return AllocationResultToString(result);
		}

		constexpr AllocationResult GetResult() const noexcept
		{
			return result;
		}

	private:
		AllocationResult result;
		std::source_location location;
	};
};
