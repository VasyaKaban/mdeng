#pragma once

#include "VulkanInclude.hpp"
#include "hrs/error.hpp"


namespace FireLand
{
	struct ImageFormat
	{
		vk::Image image;
		vk::Format format;
	};

	constexpr bool IsBadExtent(const vk::Extent2D extent) noexcept
	{
		return (extent.width == 0 || extent.height == 0);
	}
};

#include <tuple>
#include <optional>
#include <ranges>
#include "hrs/enum_meta.hpp"


template<auto ...args>
struct TupleFromArgs
{
	using type = std::tuple<decltype(args)...>;
};

template<auto ...args>
using TupleFromArgsT = TupleFromArgs<args...>::type;

template<std::size_t N>
struct StaticString
{
	char data[N];

	constexpr static std::size_t SIZE = N;

	constexpr StaticString(const char (&str)[N]) noexcept
	{
		for(std::size_t i = 0; i < N; i++)
			data[i] = str[i];
	}
};


constexpr auto

template<std::size_t N>
StaticString(const char (&str)[N]) -> StaticString<N>;

template<std::size_t N>
constexpr auto foo2(const char(&seq)[N]) noexcept
{
	StaticString str(seq);
	std::array<std::size_t, hrs::detail::token_count_v<std::ranges::count(seq, ',')>> arr;
	for(std::size_t i = 0; i < str.SIZE; i++)
	{
		if(str.data[i] == ',')
			str.data[i] = '\0';

	}

	static_assert(std::is_constant_evaluated());

	return str;
}

constexpr auto str = foo2("Hello, World!");
constexpr auto s = str.data;

constexpr std::size_t strlen_cns(const char *str, std::size_t i) noexcept
{
	if(str[i] == '\0')
		return i;
	return strlen_cns(str, i + 1);
}

constexpr auto size = strlen_cns("sdsf", 0);

constexpr static auto foo()
{
	constexpr static auto tokens = HRS_TOKENIZE(abc, 2,5 ,6 ,5 4);
	constexpr auto create_static_str = [&]<std::size_t I>()
	{
		return StaticString<tokens[I].size() + 1>(tokens[0]);
	};

	constexpr auto str = create_static_str<0>();

	//static_assert(std::is_constant_evaluated());
	constexpr StaticString<tokens[0].size() + 1> sstr(tokens[0]);
}

template<std::size_t N>
constexpr auto bar(const std::array<std::string_view, N> &names) noexcept
{
	return [&]<std::size_t ...Indices>(const std::index_sequence<Indices...> &) noexcept
	{
		return std::tuple{StaticString<std::get<Indices>(names).size()>(std::get<Indices>(names))...};
	}(std::make_index_sequence<N>{});
}

constexpr auto foo()
{
	constexpr auto tokens = HRS_TOKENIZE(1, 2, 3);
	constexpr auto static_tokens = bar(tokens);
}

#define GEN_DEVICE_LOADER(NAME, ...) \
struct NAME \
{ \
	constexpr static TupleFromArgsT<__VA_ARGS__> functions; \
	constexpr static auto names = HRS_TOKENIZE(__VA_ARGS__); \
\
	template<std::size_t I, typename F> \
	bool iterate(F &&f, VkDevice device, PFN_vkGetDeviceProcAddr get_proc_addr) noexcept \
	{ \
		using function_t = std::tuple_element_t<I, decltype(functions)>; \
		std::string_view name = std::get<I>(names); \
		auto &func = std::get<I>(functions); \
		void *ptr = get_proc_addr(device, name); \
		if(ptr) \
		{ \
			std::get<I>(functions) = reinterpret_cast<function_t>(ptr)>; \
		} \
		else \
		{ \
			bool res = std::forward<F>(f)(reinterpret_cast<void *>(std::get<I>(functions)), std::get<I>(names)); \
			if(!res) \
				return res; \
		} \
		if constexpr(I == std::tuple_size_v<decltype(functions)> - 1) \
			return true; \
		else \
			return iterate<I + 1>(std::forward<F>(f), device, get_proc_addr); \
	} \
\
	template<typename F> \
		requires std::is_invocable_r_v<bool, F, void *, std::string_view> \
		bool Init(F &&f, VkDevice device, PFN_vkGetDeviceProcAddr get_proc_addr) noexcept \
	{ \
		return iterate<0>(std::forward<F>(f), device, get_proc_addr); \
	} \
\
	template<typename F> \
	constexpr auto get() const noexcept \
	{ \
		return std::get<F>(functions); \
	} \
\
	template<typename F, typename ...Args> \
	constexpr auto operator()(Args &&...args) const noexcept \
	{ \
		return get<F>(std::forward<Args>(args)...); \
	} \
};

using Loader = GEN_DEVICE_LOADER(LOADER_COOL, vkQueueSubmit, vkCreateFence);

#define DECLARE_VK_FUNCTION(NAME) PFN_##NAME NAME

#define RETRIEVE_VK_DEVICE_FUNCTION(DEVICE, NAME, CHECK) \
{ \
	PFN_vkVoidFunction void_func = vkGetDeviceProcAddr(DEVICE, #NAME); \
	if(!void_func) \
	{ \
		if(!CHECK(void_func, #NAME)) \
			return false; \
	} \
\
	NAME = reinterpret_cast<PFN_##NAME>(void_func); \
}

#include <vulkan/vulkan.h>

#define RETRIEVE_VK_DEVICE_FUNCTION_EX(NAME) RETRIEVE_VK_DEVICE_FUNCTION(device, NAME, std::forward<F>(check))



#define FOR_EACH(V, ...) EXPAND(V)

struct VkDeviceLoader
{

	DECLARE_VK_FUNCTION(vkQueueSubmit);
	DECLARE_VK_FUNCTION(vkCreateFence);

	template<typename F>
		requires std::is_invocable_r_v<bool, F, PFN_vkVoidFunction, std::string_view>
	bool Init(VkDevice device, F &&check) noexcept
	{
		RETRIEVE_VK_DEVICE_FUNCTION_EX(vkQueueSubmit)
		RETRIEVE_VK_DEVICE_FUNCTION_EX(vkCreateFence)

		return true;
	}
};

auto foo()
{
	vkQueueSubmit()
	PFN_vkGetInstanceProcAddr a;
	PFN_vkGetDeviceProcAddr a;
}

template<>
struct hrs::enum_error_traits<vk::Result>
{
	constexpr static void traits_hint() noexcept {};
	constexpr static std::string_view get_name(vk::Result value) noexcept
	{
		switch (value)
		{
			case vk::Result::eSuccess: return "Success";
			case vk::Result::eNotReady: return "NotReady";
			case vk::Result::eTimeout: return "Timeout";
			case vk::Result::eEventSet: return "EventSet";
			case vk::Result::eEventReset: return "EventReset";
			case vk::Result::eIncomplete: return "Incomplete";
			case vk::Result::eErrorOutOfHostMemory: return "ErrorOutOfHostMemory";
			case vk::Result::eErrorOutOfDeviceMemory: return "ErrorOutOfDeviceMemory";
			case vk::Result::eErrorInitializationFailed: return "ErrorInitializationFailed";
			case vk::Result::eErrorDeviceLost: return "ErrorDeviceLost";
			case vk::Result::eErrorMemoryMapFailed: return "ErrorMemoryMapFailed";
			case vk::Result::eErrorLayerNotPresent: return "ErrorLayerNotPresent";
			case vk::Result::eErrorExtensionNotPresent: return "ErrorExtensionNotPresent";
			case vk::Result::eErrorFeatureNotPresent: return "ErrorFeatureNotPresent";
			case vk::Result::eErrorIncompatibleDriver: return "ErrorIncompatibleDriver";
			case vk::Result::eErrorTooManyObjects: return "ErrorTooManyObjects";
			case vk::Result::eErrorFormatNotSupported: return "ErrorFormatNotSupported";
			case vk::Result::eErrorFragmentedPool: return "ErrorFragmentedPool";
			case vk::Result::eErrorUnknown: return "ErrorUnknown";
			case vk::Result::eErrorOutOfPoolMemory: return "ErrorOutOfPoolMemory";
			case vk::Result::eErrorInvalidExternalHandle: return "ErrorInvalidExternalHandle";
			case vk::Result::eErrorFragmentation: return "ErrorFragmentation";
			case vk::Result::eErrorInvalidOpaqueCaptureAddress: return "ErrorInvalidOpaqueCaptureAddress";
			case vk::Result::ePipelineCompileRequired: return "PipelineCompileRequired";
			case vk::Result::eErrorSurfaceLostKHR: return "ErrorSurfaceLostKHR";
			case vk::Result::eErrorNativeWindowInUseKHR: return "ErrorNativeWindowInUseKHR";
			case vk::Result::eSuboptimalKHR: return "SuboptimalKHR";
			case vk::Result::eErrorOutOfDateKHR: return "ErrorOutOfDateKHR";
			case vk::Result::eErrorIncompatibleDisplayKHR: return "ErrorIncompatibleDisplayKHR";
			case vk::Result::eErrorValidationFailedEXT: return "ErrorValidationFailedEXT";
			case vk::Result::eErrorInvalidShaderNV: return "ErrorInvalidShaderNV";
			case vk::Result::eErrorImageUsageNotSupportedKHR: return "ErrorImageUsageNotSupportedKHR";
			case vk::Result::eErrorVideoPictureLayoutNotSupportedKHR: return "ErrorVideoPictureLayoutNotSupportedKHR";
			case vk::Result::eErrorVideoProfileOperationNotSupportedKHR: return "ErrorVideoProfileOperationNotSupportedKHR";
			case vk::Result::eErrorVideoProfileFormatNotSupportedKHR: return "ErrorVideoProfileFormatNotSupportedKHR";
			case vk::Result::eErrorVideoProfileCodecNotSupportedKHR: return "ErrorVideoProfileCodecNotSupportedKHR";
			case vk::Result::eErrorVideoStdVersionNotSupportedKHR: return "ErrorVideoStdVersionNotSupportedKHR";
			case vk::Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT: return "ErrorInvalidDrmFormatModifierPlaneLayoutEXT";
			case vk::Result::eErrorNotPermittedKHR: return "ErrorNotPermittedKHR";
#if defined( VK_USE_PLATFORM_WIN32_KHR )
			case vk::Result::eErrorFullScreenExclusiveModeLostEXT: return "ErrorFullScreenExclusiveModeLostEXT";
#endif /*VK_USE_PLATFORM_WIN32_KHR*/
			case vk::Result::eThreadIdleKHR: return "ThreadIdleKHR";
			case vk::Result::eThreadDoneKHR: return "ThreadDoneKHR";
			case vk::Result::eOperationDeferredKHR: return "OperationDeferredKHR";
			case vk::Result::eOperationNotDeferredKHR: return "OperationNotDeferredKHR";
			case vk::Result::eErrorInvalidVideoStdParametersKHR: return "ErrorInvalidVideoStdParametersKHR";
			case vk::Result::eErrorCompressionExhaustedEXT: return "ErrorCompressionExhaustedEXT";
			case vk::Result::eErrorIncompatibleShaderBinaryEXT: return "ErrorIncompatibleShaderBinaryEXT";
			default: return "";
		}
	}
};
