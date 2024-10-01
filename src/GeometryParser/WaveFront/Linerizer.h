#pragma once

#include "ObjData.h"

namespace GeometryParser
{
    class Linerizer
    {
    public:
        constexpr static std::size_t COMPONENTS = 2;

        Linerizer(ObjData& _obj_data) noexcept;
        ~Linerizer() = default;
        Linerizer(const Linerizer&) = default;
        Linerizer(Linerizer&&) noexcept = default;
        Linerizer& operator=(const Linerizer&) = default;
        Linerizer& operator=(Linerizer&&) noexcept = default;

        void Reset(ObjData& _obj_data) noexcept;

        explicit operator bool() const noexcept;

        std::optional<std::array<std::size_t, 2>> Next() noexcept;
    private:
        ObjData* obj_data;
        std::size_t index;
    };
};
