#pragma once

#include "VulkanInclude.hpp"
#include "../hrs/error.hpp"

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
