#include "RenderPass.h"
#include "RenderWorld.h"
#include "../../Context/Device.h"
#include "Shader.h"
#include "MaterialGroup.h"

namespace FireLand
{
	RenderPass::RenderPass(RenderWorld *_parent_world,
						   std::vector<vk::CommandBuffer> &&_command_buffers,
						   vk::CommandPool _command_pool) noexcept
		: parent_world(_parent_world),
		  command_buffers(std::move(_command_buffers)),
		  command_pool(_command_pool) {}

	RenderPass::~RenderPass()
	{
		destroy();
	}

	RenderPass::RenderPass(RenderPass &&rpg) noexcept
		: parent_world(rpg.parent_world),
		  command_buffers(std::move(rpg.command_buffers)),
		  command_pool(std::exchange(rpg.command_pool, VK_NULL_HANDLE)),
		  subpass_shaders(std::move(rpg.subpass_shaders)),
		  shaders_search(std::move(rpg.shaders_search)) {}

	RenderPass & RenderPass::operator=(RenderPass &&rpg) noexcept
	{
		destroy();

		parent_world = rpg.parent_world;
		command_buffers = std::move(rpg.command_buffers);
		command_pool = std::exchange(rpg.command_pool, VK_NULL_HANDLE);
		subpass_shaders = std::move(rpg.subpass_shaders);
		shaders_search = std::move(rpg.shaders_search);

		return *this;
	}

	std::optional<std::uint32_t> RenderPass::FindSubpass(const Shader *shader) const noexcept
	{
		auto it = shaders_search.find(shader);
		if(it != shaders_search.end())
			return it->second.subpass;

		return {};
	}

	void RenderPass::RemoveShader(const Shader * shader) noexcept
	{
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return;

		subpass_shaders[it->second.subpass].erase(it->second.it);
	}

	std::pair<vk::CommandBuffer, vk::Result>
	RenderPass::Render(std::uint32_t frame_index, vk::DescriptorSet globals_set)
	{
		if(!GetState())
			return {VK_NULL_HANDLE, vk::Result::eSuccess};

		vk::CommandBuffer command_buffer = command_buffers[frame_index];
		const vk::CommandBufferBeginInfo info(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		auto begin_res = command_buffer.begin(info);
		if(begin_res != vk::Result::eSuccess)
			return {command_buffer, begin_res};

		vk::DescriptorSet descriptor_set = GetDescriptorSet(frame_index);
		for(std::uint32_t i = 0; i < GetSubpassCount(); i++)
		{
			if(i == 0)
				Start(command_buffer);
			else
				NextSubpass(command_buffer);

			for(const auto &shader : subpass_shaders[i])
			{
				const auto inheritance_info = GetInheritanceInfo();
				auto [shader_command_buffer, shader_res] = shader.second->Render(frame_index,
																				 globals_set,
																				 descriptor_set,
																				 inheritance_info);
				if(shader_res != vk::Result::eSuccess)
					return {command_buffer, shader_res};

				if(shader_command_buffer)
					command_buffer.executeCommands(shader_command_buffer);
			}
		}

		End(command_buffer);
		return {command_buffer, command_buffer.end()};
	}

	hrs::error RenderPass::Flush(std::uint32_t frame_index)
	{
		for(std::uint32_t i = 0; i < GetSubpassCount(); i++)
		{
			for(auto &shader : subpass_shaders[i])
			{
				auto err = shader.second->Flush(frame_index);
				if(err)
					return err;
			}
		}

		return FlushInner(frame_index);
	}

	RenderWorld * RenderPass::GetParentWorld() noexcept
	{
		return parent_world;
	}

	const RenderWorld * RenderPass::GetParentWorld() const noexcept
	{
		return parent_world;
	}

	vk::Result RenderPass::RebindMaterial(const Shader *shader, const Material *mtl)
	{
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return vk::Result::eSuccess;

		return it->second.it->second->RebindMaterial(mtl);
	}

	vk::Result RenderPass::RebindMaterial(const MaterialGroup *mtl_group)
	{
		const Shader *shader = mtl_group->GetParentShader();
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return vk::Result::eSuccess;

		return it->second.it->second->RebindMaterial(mtl_group);
	}

	void RenderPass::NotifyNewShaderObjectData(const Shader *shader,
											   Data data,
											   std::uint32_t *index_subscriber_ptr)
	{
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return;

		it->second.it->second->AddObjectData(data, index_subscriber_ptr);
	}

	void RenderPass::NotifyUpdateShaderObjectData(const Shader *shader,
												  Data data,
												  std::uint32_t index,
												  const hrs::block<vk::DeviceSize> &data_block,
												  vk::DeviceSize in_data_buffer_offset)
	{
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return;

		it->second.it->second->UpdateObjectData(data, index, data_block, in_data_buffer_offset);
	}

	void RenderPass::NotifyRemoveShaderObjectData(const Shader *shader,
												  std::uint32_t index)
	{
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return;

		it->second.it->second->RemoveObjectData(index);
	}

	void RenderPass::NotifyNewRenderGroupInstance(const RenderGroup *render_group,
												  std::uint32_t data_index,
												  std::uint32_t *subscriber_ptr)
	{
		const MaterialGroup *material_group = render_group->GetParentMaterialGroup();
		const Shader *shader = material_group->GetParentShader();
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return;

		it->second.it->second->NotifyNewRenderGroupInstance(render_group, data_index, subscriber_ptr);
	}

	void RenderPass::NotifyRemoveRenderGroupInstance(const RenderGroup *render_group,
													 std::uint32_t index)
	{
		const MaterialGroup *material_group = render_group->GetParentMaterialGroup();
		const Shader *shader = material_group->GetParentShader();
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return;

		it->second.it->second->NotifyRemoveRenderGroupInstance(render_group, index);
	}

	RenderGroup * RenderPass::AddRenderGroup(const Shader *shader,
											 const Material *mtl,
											 const Mesh *mesh,
											 std::uint32_t init_size_power,
											 std::uint32_t rounding_size,
											 bool _enabled)
	{
		auto it = shaders_search.find(shader);
		if(it == shaders_search.end())
			return nullptr;

		return it->second.it->second->AddRenderGroup(mtl,
													 mesh,
													 init_size_power,
													 rounding_size,
													 _enabled);
	}

	void RenderPass::destroy() noexcept
	{
		if(!parent_world)
			return;

		vk::Device device_handle = parent_world->GetParentDevice()->GetHandle();
		subpass_shaders.clear();
		shaders_search.clear();
		device_handle.destroy(command_pool);
	}

	std::pair<RenderPass::SubpassShaderBinding::iterator, bool>
	RenderPass::add_shader(Shader *shader,
						   std::uint32_t subpass,
						   std::size_t priority)
	{
		auto search_it = shaders_search.find(shader);
		if(search_it != shaders_search.end())
			return {{}, false};

		auto it = subpass_shaders[subpass].emplace(priority, shader);
		shaders_search.insert({shader, ShaderSearch{.it = it, .subpass = subpass}});
		return {it, true};
	}

	hrs::expected<std::vector<vk::CommandBuffer>, vk::Result>
	RenderPass::allocate_secondary_command_buffers() noexcept
	{
		std::uint32_t frames = parent_world->GetFrameCount();
		const vk::CommandBufferAllocateInfo info(command_pool,
												 vk::CommandBufferLevel::eSecondary,
												 frames);
		auto [buffers_res, buffers] =
			parent_world->GetParentDevice()->GetHandle().allocateCommandBuffers(info);
		if(buffers_res != vk::Result::eSuccess)
			return buffers_res;

		return buffers;
	}

	void RenderPass::shader_free_command_buffers(std::span<const vk::CommandBuffer> shader_buffers) noexcept
	{
		if(shader_buffers.empty())
			return;

		parent_world->GetParentDevice()->GetHandle().free(command_pool, shader_buffers);
	}
};
