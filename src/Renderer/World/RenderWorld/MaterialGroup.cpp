#include "MaterialGroup.h"
#include "RenderGroup.h"
#include "Shader.h"

namespace FireLand
{
	MaterialGroup::MaterialGroup(Shader *_parent_shader, const Material *_material, bool _enabled) noexcept
		: PlainStateful(_enabled),
		  parent_shader(_parent_shader),
		  material(_material) {}

	MaterialGroup::~MaterialGroup()
	{
		render_groups.clear();
	}

	MaterialGroup::MaterialGroup(MaterialGroup &&mtl) noexcept
		: PlainStateful(std::move(mtl)),
		  parent_shader(mtl.parent_shader),
		  material(mtl.material),
		  render_groups(std::move(mtl.render_groups)),
		  render_groups_search(std::move(mtl.render_groups_search)) {}

	MaterialGroup & MaterialGroup::operator=(MaterialGroup &&mtl) noexcept
	{
		this->~MaterialGroup();

		PlainStateful::operator=(std::move(mtl));
		parent_shader = mtl.parent_shader;
		material = mtl.material;
		render_groups = std::move(mtl.render_groups);
		render_groups_search = std::move(mtl.render_groups_search);

		return *this;
	}

	RenderGroup & MaterialGroup::AddRenderGroup(const Mesh *mesh,
												std::uint32_t init_size_power,
												std::uint32_t rounding_size,
												bool _enabled)
	{
		auto it = render_groups.find(mesh);
		if(it != render_groups.end())
			return it->second;

		DataIndexStorage *storage = parent_shader->material_group_get_data_index_storage();
		auto in_it = render_groups.insert({mesh, RenderGroup(this, mesh,
															 IndexPool(storage,
																	   init_size_power,
																	   rounding_size), _enabled)});

		return in_it.first->second;
	}

	void MaterialGroup::RemoveMesh(const Mesh *mesh) noexcept
	{
		render_groups.erase(mesh);
	}

	RenderGroup * MaterialGroup::FindRenderGroup(const Mesh *mesh) noexcept
	{
		auto it = render_groups.find(mesh);
		return (it == render_groups.end() ? nullptr : &it->second);
	}

	const RenderGroup * MaterialGroup::FindRenderGroup(const Mesh *mesh) const noexcept
	{
		auto it = render_groups.find(mesh);
		return (it == render_groups.end() ? nullptr : &it->second);
	}

	Shader * MaterialGroup::GetParentShader() noexcept
	{
		return parent_shader;
	}

	const Shader * MaterialGroup::GetParentShader() const noexcept
	{
		return parent_shader;
	}

	const Material * MaterialGroup::GetMaterial() const noexcept
	{
		return material;
	}

	void MaterialGroup::Flush()
	{
		for(auto &render_group : render_groups)
			render_group.second.Sync();
	}

	void MaterialGroup::NotifyNewRenderGroupInstance(const RenderGroup *render_group,
													 std::uint32_t data_index,
													 std::uint32_t *subscriber_ptr)
	{
		auto it = render_groups_search.find(render_group);
		if(it == render_groups_search.end())
			return;

		it->second->second.AcquireIndex(data_index, subscriber_ptr);
	}

	void MaterialGroup::NotifyRemoveRenderGroupInstance(const RenderGroup *render_group,
														std::uint32_t index)
	{
		auto it = render_groups_search.find(render_group);
		if(it == render_groups_search.end())
			return;

		it->second->second.RemoveIndex(index);
	}

	const std::map<const Mesh *, RenderGroup> & MaterialGroup::shader_get_render_groups() const noexcept
	{
		return render_groups;
	}
};
