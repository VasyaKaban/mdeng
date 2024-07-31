#pragma once
#include "codegen/InstanceLoader_gen.h"
#include <optional>
#include "../Vulkan/VulkanInclude.h"
#include "../Vulkan/LoaderInitResult.h"

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_decl.h"
};

#undef INSTANCE_LOADER_DEBUG_FUNCTIONS
#undef LOADER_GEN_LIST
