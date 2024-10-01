#pragma once

#include "Stateful.h"

namespace FireLand
{
    class PlainStateful : public Stateful
    {
    public:
        PlainStateful(bool _enabled) noexcept;
        ~PlainStateful() = default;
        PlainStateful(const PlainStateful&) = default;
        PlainStateful& operator=(const PlainStateful&) = default;

        virtual bool GetState() const noexcept override;
        virtual void SwitchState() noexcept override;
        virtual void SetState(bool _enabled) noexcept override;
    private:
        bool enabled;
    };
};
