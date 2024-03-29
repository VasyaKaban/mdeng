#pragma once

#include "Allocator.h"

namespace FireLand
{
	struct MemoryPropertyOpFlags
	{
		vk::MemoryPropertyFlags memory_property;
		MemoryTypeSatisfyOp op;
		hrs::flags<AllocationFlags> flags;

		constexpr MemoryPropertyOpFlags(vk::MemoryPropertyFlags _memory_property = {},
										MemoryTypeSatisfyOp _op = {},
										hrs::flags<AllocationFlags> _flags = {}) noexcept
			: memory_property(_memory_property),
			  op(_op),
			  flags(_flags) {}
	};

	hrs::expected<BoundedBuffer, hrs::error>
	AllocateFromMany(Allocator &allocator,
					 std::span<const MemoryPropertyOpFlags> variants,
					 const vk::BufferCreateInfo &info,
					 vk::DeviceSize alignment = 1,
					 const std::function<NewPoolSizeCalculator> &calc = MemoryType::DefaultNewPoolSizeCalculator)
	{
		if(info.size == 0 || variants.empty())
			return BoundedBuffer{};

		hrs::error err;
		for(const auto &var : variants)
		{
			auto alloc_exp = allocator.Create(info,
											  var.op,
											  var.memory_property,
											  alignment,
											  var.flags,
											  calc);

			if(alloc_exp)
				return std::move(alloc_exp.value());
			else
			{
				err = alloc_exp.error();
				if(err.holds<vk::Result>())
				{
					vk::Result res = err.revive<vk::Result>();
					switch(res)
					{
						case vk::Result::eErrorOutOfDeviceMemory:
						case vk::Result::eErrorOutOfHostMemory:
						case vk::Result::eErrorMemoryMapFailed:
							break;
						default:
							return err;
							break;
					}
				}
			}
		}

		return err;
	}

	hrs::expected<BoundedImage, hrs::error>
	AllocateFromMany(Allocator &allocator,
					 std::span<const MemoryPropertyOpFlags> variants,
					 const vk::ImageCreateInfo &info,
					  vk::DeviceSize alignment = 1,
					 const std::function<NewPoolSizeCalculator> &calc = MemoryType::DefaultNewPoolSizeCalculator)
	{
		if(variants.empty())
			return BoundedImage{};

		hrs::error err;
		for(const auto &var : variants)
		{
			auto alloc_exp = allocator.Create(info,
											  var.op,
											  var.memory_property,
											  alignment,
											  var.flags,
											  calc);

			if(alloc_exp)
				return std::move(alloc_exp.value());
			else
			{
				err = alloc_exp.error();
				if(err.holds<vk::Result>())
				{
					vk::Result res = err.revive<vk::Result>();
					switch(res)
					{
						case vk::Result::eErrorOutOfDeviceMemory:
						case vk::Result::eErrorOutOfHostMemory:
						case vk::Result::eErrorMemoryMapFailed:
							break;
						default:
							return err;
							break;
					}
				}
			}
		}

		return err;
	}
};
