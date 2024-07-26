#pragma once

#include <concepts>
#include <utility>
#include "hrs/expected.hpp"
#include "hrs/error.hpp"
#include "hrs/instantiation.hpp"

namespace FireLand
{
	class Context;

	struct DeviceUtilizer
	{
		virtual ~DeviceUtilizer();
	};

	template<typename DU, typename ...Args>
	concept DeviceUtilizerCreatable =
		std::derived_from<DU, DeviceUtilizer> &&
		std::move_constructible<DU> &&
			requires(Context *ctx, Args &&...args)
			{
				{DU::Create(ctx, std::forward<Args>(args)...)} -> hrs::type_instantiation<hrs::expected>;
				requires std::same_as<typename decltype(DU::Create(ctx, std::forward<Args>(args)...))::value_type, DU>;
				requires std::convertible_to<typename decltype(DU::Create(ctx, std::forward<Args>(args)...))::error_type,
									hrs::error>;
			};
};
