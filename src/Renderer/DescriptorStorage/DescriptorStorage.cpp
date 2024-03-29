#include "DescriptorStorage.h"
#include "../Context/Device.h"
#include "DescriptorPool.h"

namespace FireLand
{
	DescriptorStorage::DescriptorStorage(Device *_parent_device,
										vk::DescriptorSetLayout _descriptor_set_layout,
										std::uint32_t set_layout_count,
										const DescriptorPoolInfo &_pool_info)
		: parent_device(_parent_device),
		  descriptor_set_layouts(set_layout_count, _descriptor_set_layout),
		  pool_info(_pool_info) {}

	DescriptorStorage::DescriptorStorage(Device *_parent_device,
										 vk::DescriptorSetLayout _descriptor_set_layout,
										 std::uint32_t set_layout_count,
										 DescriptorPoolInfo &&_pool_info)
		: parent_device(_parent_device),
		  descriptor_set_layouts(set_layout_count, _descriptor_set_layout),
		  pool_info(std::move(_pool_info)) {}

	hrs::expected<DescriptorStorage, vk::Result>
	DescriptorStorage::Create(Device *_parent_device,
							  const vk::DescriptorSetLayoutCreateInfo &set_layout_info,
							  std::uint32_t set_layout_count,
							  const DescriptorPoolInfo &_pool_info)
	{
		if(!_parent_device)
			return DescriptorStorage(_parent_device, VK_NULL_HANDLE, set_layout_count, _pool_info);

		auto [layout_res, layout] = _parent_device->GetHandle().createDescriptorSetLayout(set_layout_info);
		if(layout_res != vk::Result::eSuccess)
			return layout_res;

		return DescriptorStorage(_parent_device, layout, set_layout_count, _pool_info);
	}

	hrs::expected<DescriptorStorage, vk::Result>
	DescriptorStorage::Create(Device *_parent_device,
							  const vk::DescriptorSetLayoutCreateInfo &set_layout_info,
							  std::uint32_t set_layout_count,
							  DescriptorPoolInfo &&_pool_info) noexcept
	{
		if(!_parent_device)
			return DescriptorStorage(_parent_device, VK_NULL_HANDLE, set_layout_count, _pool_info);

		auto [layout_res, layout] = _parent_device->GetHandle().createDescriptorSetLayout(set_layout_info);
		if(layout_res != vk::Result::eSuccess)
			return layout_res;

		return DescriptorStorage(_parent_device, layout, set_layout_count, std::move(_pool_info));
	}

	DescriptorStorage::~DescriptorStorage()
	{
		Destroy();
	}

	DescriptorStorage::DescriptorStorage(DescriptorStorage &&storage) noexcept
		: parent_device(storage.parent_device),
		  descriptor_set_layouts(std::move(storage.descriptor_set_layouts)),
		  pool_info(std::move(storage.pool_info)),
		  pools(std::move(storage.pools)) {}

	DescriptorStorage & DescriptorStorage::operator=(DescriptorStorage &&storage) noexcept
	{
		Destroy();

		parent_device = storage.parent_device;
		descriptor_set_layouts = std::move(storage.descriptor_set_layouts);
		pool_info = std::move(storage.pool_info);
		pools = std::move(storage.pools);

		return *this;
	}

	void DescriptorStorage::Destroy()
	{
		if(!IsCreated())
			return;

		pools.clear();
		vk::Device device_handle = parent_device->GetHandle();
		for(auto layout : descriptor_set_layouts)
			device_handle.destroyDescriptorSetLayout(layout);

		descriptor_set_layouts.clear();
	}

	bool DescriptorStorage::IsCreated() const noexcept
	{
		return !descriptor_set_layouts.empty() && descriptor_set_layouts[0];
	}

	hrs::expected<DescriptorSetGroup, vk::Result> DescriptorStorage::AllocateSetGroup()
	{
		for(auto &pool : pools)
		{
			auto set_exp = pool.AllocateSets(descriptor_set_layouts);
			if(set_exp)
				return DescriptorSetGroup(std::move(set_exp.value()), &pool);
			else
			{
				switch(set_exp.error())
				{
					case vk::Result::eErrorFragmentedPool:
					case vk::Result::eErrorOutOfPoolMemory:
						break;
					default:
						return set_exp.error();
						break;
				}
			}
		}

		const vk::DescriptorPoolCreateInfo info(pool_info.flags, pool_info.max_sets, pool_info.pool_sizes);
		auto pool_exp = DescriptorPool::Create(this, info);
		if(!pool_exp)
			return pool_exp.error();

		auto set_exp = pool_exp.value().AllocateSets(descriptor_set_layouts);
		if(!set_exp)
		{
			pool_exp.value().Destroy();
			return set_exp.error();
		}

		pools.push_back(std::move(pool_exp.value()));
		return DescriptorSetGroup(std::move(set_exp.value()), &*std::prev(pools.end()));
	}

	void DescriptorStorage::RetireSetGroup(const DescriptorSetGroup &group)
	{
		group.parent_pool->RetireSets(group.sets);
	}

	void DescriptorStorage::FreeSetGroup(const DescriptorSetGroup &group)
	{
		group.parent_pool->FreeSets(group.sets);
	}

	void DescriptorStorage::DestroyFoolPools() noexcept
	{
		std::erase_if(pools, [](const DescriptorPool &pool)
		{
			return pool.GetIssuedSetCount() == 0;
		});
	}

	Device * DescriptorStorage::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const Device * DescriptorStorage::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	vk::DescriptorSetLayout DescriptorStorage::GetDescriptorSetLayout() const noexcept
	{
		return descriptor_set_layouts[0];
	}

	const DescriptorPoolInfo & DescriptorStorage::GetPoolInfo() const noexcept
	{
		return pool_info;
	}
};
