#include "PlainStateful.h"

namespace FireLand
{
    PlainStateful::PlainStateful(bool _enabled) noexcept
        : enabled(_enabled)
    {}

    bool PlainStateful::GetState() const noexcept
    {
        return enabled;
    }

    void PlainStateful::SwitchState() noexcept
    {
        enabled = !enabled;
    }

    void PlainStateful::SetState(bool _enabled) noexcept
    {
        enabled = _enabled;
    }
};
