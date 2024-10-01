#pragma once

namespace FireLand
{
    enum class AllocatorResult
    {
        Success,
        MemoryPoolNotEnoughMemory,
        NoSatisfiedMemoryTypes
    };
};

#include "hrs/meta/enum_meta.hpp"
#include "hrs/meta/enum_meta_def.hpp"

HRS_REFL_ENUM_BEGIN_EXISTED(FireLand::AllocatorResult,
                            Success,
                            MemoryPoolNotEnoughMemory,
                            NoSatisfiedMemoryTypes)
HRS_REFL_ENUM_BODY()
HRS_REFL_ENUM_END()

#include "hrs/meta/enum_meta_undef.hpp"
