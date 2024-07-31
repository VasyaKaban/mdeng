#pragma once

#include "codegen/AllocatorLoader_gen.h"
#include "../Vulkan/VulkanInclude.h"
#include "../Vulkan/LoaderInitResult.h"

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_decl.h"
};

#undef LOADER_GEN_LIST
