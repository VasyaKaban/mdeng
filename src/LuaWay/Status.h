#pragma once

#include <lua.hpp>
#include <string>

namespace LuaWay
{
    enum class StatusCode
    {
        Success = 0,
        RuntimeError = 2,
        SyntaxError = 3,
        MemoryError = 4,
        InnerError = 5,
        FileError = 6
    };

    struct Status
    {
        StatusCode code;
        std::string message;

        Status();
        Status(StatusCode _code, const char* _message);
        ~Status() = default;
        Status(const Status&) = default;
        Status(Status&&) = default;
        Status& operator=(const Status&) = default;
        Status& operator=(Status&&) = default;

        bool IsSuccess() const noexcept;
        bool IsError() const noexcept;
    };
};
