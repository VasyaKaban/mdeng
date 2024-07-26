#include "GlobalLoader.h"
#include "codegen/GlobalLoader_gen.h"
#include <bit>

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_def.h"
};

#undef LOADER_GEN_TYPE
#undef LOADER_GEN_LIST
