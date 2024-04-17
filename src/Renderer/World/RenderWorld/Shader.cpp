#include "Shader.h"
#include "MaterialGroup.h"
#include "RenderPass.h"

namespace FireLand
{
	Shader::Shader(RenderPass *_parent_renderpass,
				   std::vector<vk::CommandBuffer> &&_command_buffers,
				   DataBuffer &&_data_buffer,
				   DataIndexStorage &&_data_index_storage,
				   DescriptorStorage &&_descriptor_storage) noexcept
		: parent_renderpass(_parent_renderpass),
		  command_buffers(std::move(_command_buffers)),
		  data_buffer(std::move(_data_buffer)),
		  data_index_storage(std::move(_data_index_storage)),
		  descriptor_storage(std::move(_descriptor_storage)) {}

	Shader::~Shader()
	{
		destroy();
	}

	Shader::Shader(Shader &&s) noexcept
		: parent_renderpass(s.parent_renderpass),
		  command_buffers(std::move(s.command_buffers)),
		  data_buffer(std::move(s.data_buffer)),
		  data_index_storage(std::move(s.data_index_storage)),
		  descriptor_storage(std::move(s.descriptor_storage)),
		  material_group_binding(std::move(s.material_group_binding)),
		  materials_search(std::move(s.materials_search)) {}

	Shader & Shader::operator=(Shader &&s) noexcept
	{
		destroy();

		parent_renderpass = s.parent_renderpass;
		command_buffers = std::move(s.command_buffers);
		data_buffer = std::move(s.data_buffer);
		data_index_storage = std::move(s.data_index_storage);
		descriptor_storage = std::move(s.descriptor_storage);
		material_group_binding = std::move(s.material_group_binding);
		materials_search = std::move(s.materials_search);

		return *this;
	}

	RenderPass * Shader::GetParentRenderPass() noexcept
	{
		return parent_renderpass;
	}

	const RenderPass * Shader::GetParentRenderPass() const noexcept
	{
		return parent_renderpass;
	}

	MaterialGroup * Shader::FindMaterialGroup(const Material *mtl) noexcept
	{
		auto it = materials_search.find(mtl);
		return (it == materials_search.end() ? nullptr : &*it->second.material_group_it);
	}

	const MaterialGroup * Shader::FindMaterialGroup(const Material *mtl) const noexcept
	{
		auto it = materials_search.find(mtl);
		return (it == materials_search.end() ? nullptr : &*it->second.material_group_it);
	}

	void Shader::RemoveMaterial(const Material *mtl) noexcept
	{
		auto it = materials_search.find(mtl);
		if(it == materials_search.end())
			return;

		unlink_material_group(it);
	}

	vk::Result Shader::RebindMaterial(const Material *mtl)
	{
		auto it = materials_search.find(mtl);
		if(it == materials_search.end())
			return vk::Result::eSuccess;

		auto material_group = unlink_material_group(it);

		auto insert_it = material_group_binding.find(mtl);
		if(insert_it == material_group_binding.end())//no bindings
		{
			//acquire
			auto insert_it_exp = create_binding(mtl);
			if(!insert_it_exp)
				return insert_it_exp.error();

			insert_it = insert_it_exp.value();
		}

		//just insert
		insert_it->second.push_back(std::move(material_group));
		auto ret_it = std::prev(insert_it->second.end());
		MaterialSearch ms{.material_group_it = ret_it, .binding_it = insert_it};
		materials_search.insert({mtl, ms});
		return vk::Result::eSuccess;
	}

	vk::Result Shader::RebindMaterial(const MaterialGroup *mtl_group)
	{
		return RebindMaterial(mtl_group->GetMaterial());
	}

	void Shader::AddObjectData(Data data, std::uint32_t *index_subscriber_ptr)
	{
		data_buffer.NewAddOp(DataAddOp(index_subscriber_ptr, data));
	}

	void Shader::UpdateObjectData(Data data,
								  std::uint32_t index,
								  const hrs::block<vk::DeviceSize> &data_block,
								  vk::DeviceSize in_data_buffer_offset)
	{
		data_buffer.NewUpdateOp(DataUpdateOp(index, data, data_block, in_data_buffer_offset));
	}

	void Shader::RemoveObjectData(std::uint32_t index)
	{
		data_buffer.NewRemoveOp(DataRemoveOp(index));
	}

	std::pair<vk::CommandBuffer, vk::Result>
	Shader::Render(std::uint32_t frame_index,
				   vk::DescriptorSet globals_set,
				   vk::DescriptorSet renderpass_descriptor_set,
				   const vk::CommandBufferInheritanceInfo &inheritance_info) const noexcept
	{
		if(!GetState())
			return {VK_NULL_HANDLE, vk::Result::eSuccess};

		if(material_group_binding.empty())
			return {VK_NULL_HANDLE, vk::Result::eSuccess};

		vk::CommandBuffer command_buffer = command_buffers[frame_index];
		const vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit,
											  &inheritance_info);
		vk::Result begin_res = command_buffer.begin(info);
		if(begin_res != vk::Result::eSuccess)
			return {command_buffer, begin_res};

		SetPerCallData(command_buffer,
					   data_buffer.GetHandle(),
					   data_index_storage.GetBuffer(frame_index),
					   globals_set,
					   renderpass_descriptor_set);

		const Material *prev_material = nullptr;
		const Mesh *prev_mesh = nullptr;
		for(auto &binding : material_group_binding)
		{
			vk::DescriptorSet material_set = binding.first.descriptor_set_group.sets[frame_index];
			for(auto &material_group : binding.second)
			{
				if(!material_group.GetState())
					continue;

				const auto &render_groups = material_group.shader_get_render_groups();
				auto start_it = render_groups.begin();
				for(; start_it != render_groups.end(); start_it++)
					if(start_it->second.IsRenderable())
						break;

				if(start_it == render_groups.end())
					continue;

				const Material *target_material = material_group.GetMaterial();
				Bind(command_buffer,
					 material_set,
					 target_material,
					 prev_material);

				for(; start_it != render_groups.end(); start_it++)
					if(start_it->second.IsRenderable())
					{
						start_it->second.Render(prev_mesh, command_buffer);
						prev_mesh = start_it->second.GetMesh();
					}

				prev_material = target_material;
			}
		}

		vk::Result end_res = command_buffer.end();
		return {command_buffer, end_res};
	}

	hrs::error Shader::Flush(std::uint32_t frame_index)
	{
		for(auto &binding : material_group_binding)
			for(auto &material_group : binding.second)
				material_group.Flush();

		auto err = data_buffer.SyncAndWrite();
		if(err)
			return err;

		err = data_index_storage.SyncAndWrite(frame_index);
		if(err)
			return err;

		return FlushInner(frame_index);
	}

	void Shader::NotifyNewRenderGroupInstance(const RenderGroup *render_group,
											  std::uint32_t data_index,
											  std::uint32_t *subscriber_ptr)
	{
		const MaterialGroup *material_group = render_group->GetParentMaterialGroup();
		auto it = materials_search.find(material_group->GetMaterial());
		if(it == materials_search.end())
			return;

		it->second.material_group_it->NotifyNewRenderGroupInstance(render_group, data_index, subscriber_ptr);
	}

	void Shader::NotifyRemoveRenderGroupInstance(const RenderGroup *render_group,
												 std::uint32_t index)
	{
		const MaterialGroup *material_group = render_group->GetParentMaterialGroup();
		auto it = materials_search.find(material_group->GetMaterial());
		if(it == materials_search.end())
			return;

		it->second.material_group_it->NotifyRemoveRenderGroupInstance(render_group, index);
	}

	RenderGroup * Shader::AddRenderGroup(const Material *mtl,
										 const Mesh *mesh,
										 std::uint32_t init_size_power,
										 std::uint32_t rounding_size,
										 bool _enabled)
	{
		auto it = materials_search.find(mtl);
		if(it == materials_search.end())
			return nullptr;

		return &it->second.material_group_it->AddRenderGroup(mesh,
															 init_size_power,
															 rounding_size,
															 _enabled);
	}

	void Shader::destroy() noexcept
	{
		if(!parent_renderpass)
			return;

		materials_search.clear();
		material_group_binding.clear();
		descriptor_storage.Destroy();
		data_index_storage.Destroy();
		data_buffer.Destroy();
		parent_renderpass->shader_free_command_buffers(command_buffers);
	}

	hrs::expected<std::pair<Shader::MaterialSearch, bool>, vk::Result>
	Shader::add_material(Material *material, bool _enabled)
	{
		auto it = materials_search.find(material);
		if(it != materials_search.end())
			return std::pair{it->second, false};

		auto insert_it = material_group_binding.find(material);
		if(insert_it == material_group_binding.end())//no materials with same binding
		{
			//acquire
			auto insert_it_exp = create_binding(material);
			if(!insert_it_exp)
				return insert_it_exp.error();

			insert_it = insert_it_exp.value();
		}

		//no acquire
		insert_it->second.push_back(MaterialGroup(this, material, _enabled));
		auto ret_it = std::prev(insert_it->second.end());
		MaterialSearch ms{.material_group_it = ret_it, .binding_it = insert_it};
		materials_search.insert({material, ms});
		return std::pair{ms, true};
	}

	MaterialGroup Shader::unlink_material_group(MaterialGroupsSearchContainer::iterator it) noexcept
	{
		MaterialGroupsContainer &material_groups_container = it->second.binding_it->second;
		const DescriptorSetGroupKey &descriptor_set_group_key = it->second.binding_it->first;
		if(material_groups_container.size() == 1)//only one
		{
			descriptor_storage.RetireSetGroup(descriptor_set_group_key.descriptor_set_group);
			MaterialGroup out_group = std::move(it->second.binding_it->second[0]);
			material_group_binding.erase(it->second.binding_it);
			return out_group;
		}
		else//change reference material in key
		{
			MaterialGroup out_group = std::move(*it->second.material_group_it);
			if(it->second.material_group_it != std::prev(material_groups_container.end()))
				std::swap(material_groups_container.back(), *it->second.material_group_it);

			material_groups_container.pop_back();
			if(descriptor_set_group_key.material_compare == out_group.GetMaterial())
			{
				//change material key
				descriptor_set_group_key.material_compare =
					material_groups_container.back().GetMaterial();
			}

			return out_group;
		}
	}

	hrs::expected<Shader::MaterialGroupBindingsContainer::iterator, vk::Result>
	Shader::create_binding(const Material *mtl)
	{
		auto set_group_exp = descriptor_storage.AllocateSetGroup();
		if(!set_group_exp)
			return set_group_exp.error();

		mtl->WriteDescriptors(set_group_exp.value());

		std::vector<MaterialGroup> tmp_groups_vec;
		tmp_groups_vec.reserve(1);
		auto key_it = material_group_binding.insert(
			{DescriptorSetGroupKey{.descriptor_set_group = set_group_exp.value(),
								   .material_compare = mtl},
			 std::move(tmp_groups_vec)});

		return key_it.first;
	}

	DataIndexStorage * Shader::material_group_get_data_index_storage() noexcept
	{
		return &data_index_storage;
	}

	const DataIndexStorage * Shader::material_group_get_data_index_storage() const noexcept
	{
		return &data_index_storage;
	}
};
