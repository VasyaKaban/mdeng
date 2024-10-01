#pragma once
#include "../Vulkan/LoaderInitResult.h"
#include "../Vulkan/VulkanInclude.h"
#include "codegen/InstanceLoader_gen.h"
#include <optional>

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_decl.h"
};

#undef INSTANCE_LOADER_DEBUG_FUNCTIONS
#undef LOADER_GEN_LIST
