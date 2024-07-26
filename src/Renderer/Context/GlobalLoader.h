#pragma once
#include "codegen/GlobalLoader_gen.h"
#include <optional>
#include "../Vulkan/VulkanInclude.h"

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_decl.h"
};

#undef LOADER_GEN_TYPE
#undef LOADER_GEN_LIST
