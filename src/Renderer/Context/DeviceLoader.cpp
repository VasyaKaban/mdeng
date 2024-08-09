#include "DeviceLoader.h"
#include "codegen/DeviceLoader_gen.h"

namespace FireLand
{
#include "../Vulkan/codegen/loader_gen_def.h"
};

#undef LOADER_GEN_LIST

