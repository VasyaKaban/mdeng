#pragma once

#include <utility>

#define DOT_ACCESS
#define ARROW_ACCESS

#define HRS_INJECT_METHOD(access, name, obj, ...) \
HRS_INJECT_METHOD_##access(name, obj, __VA_ARGS__)

#define HRS_INJECT_METHOD_DOT_ACCESS(name, obj, ...) \
template<typename ...Args> \
constexpr auto name(Args &&...args) __VA_ARGS__ \
{ \
	return obj.name(std::forward<Args>(args)...); \
}

#define HRS_INJECT_METHOD_ARROW_ACCESS(name, obj, ...) \
template<typename ...Args> \
constexpr auto name(Args &&...args) __VA_ARGS__ \
{ \
	return obj->name(std::forward<Args>(args)...); \
}
