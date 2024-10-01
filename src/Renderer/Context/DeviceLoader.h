#pragma once
#include "../Vulkan/LoaderInitResult.h"
#include "../Vulkan/VulkanInclude.h"
#include "codegen/DeviceLoader_gen.h"
#include <optional>

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_decl.h"
};

#undef LOADER_GEN_LIST
