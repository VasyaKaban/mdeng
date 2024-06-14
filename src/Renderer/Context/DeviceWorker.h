#pragma once

namespace FireLand
{
	class Device;

	class DeviceWorker
	{
	public:
		virtual ~DeviceWorker() {}

		virtual Device * GetParentDevice() noexcept = 0;
		virtual const Device * GetParentDevice() const noexcept = 0;
	};
};
