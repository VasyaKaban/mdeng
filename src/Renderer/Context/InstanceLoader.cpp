#include "InstanceLoader.h"
#include "codegen/InstanceLoader_gen.h"
#include <bit>

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_def.h"
};

#undef INSTANCE_LOADER_DEBUG_FUNCTIONS
#undef LOADER_GEN_TYPE
#undef LOADER_GEN_LIST
