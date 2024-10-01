#pragma once

namespace FireLand
{
    struct Stateful
    {
        virtual ~Stateful() = default;
        virtual bool GetState() const noexcept = 0;
        virtual void SwitchState() noexcept = 0;
        virtual void SetState(bool _enabled) noexcept = 0;
    };
};
