#include "LoaderInitResult.h"

namespace FireLand
{
	LoaderInitResult::LoaderInitResult() noexcept
		: result(std::monostate{}) {}

	LoaderInitResult::LoaderInitResult(std::monostate) noexcept
		: result(std::monostate{}) {}

	LoaderInitResult::LoaderInitResult(std::size_t loaded_count) noexcept
		: result(loaded_count) {}

	LoaderInitResult::LoaderInitResult(std::string_view failed_name) noexcept
		: result(failed_name) {}

	bool LoaderInitResult::IsOk() const noexcept
	{
		return std::holds_alternative<std::size_t>(result);
	}

	bool LoaderInitResult::IsFailure() const noexcept
	{
		return !IsOk();
	}

	bool LoaderInitResult::IsLoaderFunctionFailure() const noexcept
	{
		return std::holds_alternative<std::monostate>(result);
	}

	bool LoaderInitResult::IsRequiredFunctionFailure() const noexcept
	{
		return std::holds_alternative<std::string_view>(result);
	}

	std::size_t LoaderInitResult::GetLoadedCount() const noexcept
	{
		return std::get<std::size_t>(result);
	}

	std::string_view LoaderInitResult::GetRequiredFailureName() const noexcept
	{
		return std::get<std::string_view>(result);
	}
};
