#pragma once

#include "../../hrs/expected.hpp"
#include "../../hrs/instantiation.hpp"

namespace FireLand
{
	class DeviceWorker
	{
	public:
		virtual ~DeviceWorker() {};
		virtual void Destroy() = 0;
	};

	template<typename W, typename ...Args>
	concept DeviceWorkerConcept =
		std::derived_from<W, DeviceWorker> &&
		std::move_constructible<W> &&
		requires(Args &&...args)
		{
			{W::Create(std::forward<Args>(args)...)} -> hrs::instantiation<hrs::expected>;
			requires std::same_as<W, typename decltype(W::Create(std::forward<Args>(args)...))::value_type>;
			requires std::move_constructible<typename decltype(W::Create(std::forward<Args>(args)...))::error_type>;
		};
};
