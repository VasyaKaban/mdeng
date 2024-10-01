#pragma once

#include "VulkanInclude.h"
#include "hrs/meta/enum_meta.hpp"

#include "hrs/meta/enum_meta_def.hpp"

HRS_REFL_ENUM_BEGIN_EXISTED(VkResult,
                            VK_SUCCESS,
                            VK_NOT_READY,
                            VK_TIMEOUT,
                            VK_EVENT_SET,
                            VK_EVENT_RESET,
                            VK_INCOMPLETE,
                            VK_ERROR_OUT_OF_HOST_MEMORY,
                            VK_ERROR_OUT_OF_DEVICE_MEMORY,
                            VK_ERROR_INITIALIZATION_FAILED,
                            VK_ERROR_DEVICE_LOST,
                            VK_ERROR_MEMORY_MAP_FAILED,
                            VK_ERROR_LAYER_NOT_PRESENT,
                            VK_ERROR_EXTENSION_NOT_PRESENT,
                            VK_ERROR_FEATURE_NOT_PRESENT,
                            VK_ERROR_INCOMPATIBLE_DRIVER,
                            VK_ERROR_TOO_MANY_OBJECTS,
                            VK_ERROR_FORMAT_NOT_SUPPORTED,
                            VK_ERROR_FRAGMENTED_POOL,
                            VK_ERROR_UNKNOWN,
                            VK_ERROR_SURFACE_LOST_KHR,
                            VK_ERROR_NATIVE_WINDOW_IN_USE_KHR,
                            VK_SUBOPTIMAL_KHR,
                            VK_ERROR_OUT_OF_DATE_KHR,
                            VK_ERROR_VALIDATION_FAILED_EXT)
HRS_REFL_ENUM_BODY()
HRS_REFL_ENUM_END()

#include "hrs/meta/enum_meta_undef.hpp"
