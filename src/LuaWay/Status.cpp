#include "Status.h"

namespace LuaWay
{
	Status::Status()
		: code(StatusCode::Success),
		  message(nullptr) {}

	Status::Status(StatusCode _code, const char *_message)
		: code(_code),
		  message(_message) {}

	bool Status::IsSuccess() const noexcept
	{
		return code == StatusCode::Success;
	}

	bool Status::IsError() const noexcept
	{
		return !IsSuccess();
	}
};
