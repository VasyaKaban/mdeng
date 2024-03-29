#include "RenderWorld.h"
#include "Shader.h"
#include "MaterialGroup.h"

namespace FireLand
{
	RenderWorld::RenderWorld(Device *_parent_device,
							 std::uint32_t _frame_count,
							 std::uint32_t _queue_family_index,
							 const std::function<NewPoolSizeCalculator> &_calc) noexcept
		: parent_device(_parent_device),
		  frame_count(_frame_count),
		  queue_family_index(_queue_family_index),
		  calc(_calc) {}

	RenderWorld::~RenderWorld()
	{
		/*Device *parent_device;
		std::uint32_t frame_count;
		std::uint32_t queue_family_index;
		std::function<NewPoolSizeCalculator> calc;
		render_results
		RenderPassesContainer renderpasses;
		RenderPassesSearchContainer renderpasses_search;*/

		renderpasses_search.clear();
		renderpasses.clear();
	}

	RenderWorld::RenderWorld(RenderWorld &&rw) noexcept
		: parent_device(rw.parent_device),
		  frame_count(rw.frame_count),
		  queue_family_index(rw.queue_family_index),
		  calc(rw.calc),
		  render_results(std::move(rw.render_results)),
		  renderpasses(std::move(rw.renderpasses)),
		  renderpasses_search(std::move(rw.renderpasses_search)) {}

	RenderWorld & RenderWorld::operator=(RenderWorld &&rw) noexcept
	{
		this->~RenderWorld();

		parent_device = rw.parent_device;
		frame_count = rw.frame_count;
		queue_family_index = rw.queue_family_index;
		calc = rw.calc;
		render_results = std::move(rw.render_results);
		renderpasses = std::move(rw.renderpasses);
		renderpasses_search = std::move(rw.renderpasses_search);

		return *this;
	}

	vk::Result RenderWorld::RebindMaterial(const Shader *shader, const Material *mtl)
	{
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return vk::Result::eSuccess;

		return it->second->second->RebindMaterial(shader, mtl);
	}

	vk::Result RenderWorld::RebindMaterial(const MaterialGroup *mtl_group)
	{
		const Shader *shader = mtl_group->GetParentShader();
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return vk::Result::eSuccess;

		return it->second->second->RebindMaterial(mtl_group);
	}

	void RenderWorld::NotifyNewShaderObjectData(const Shader *shader,
												Data data,
												std::uint32_t *index_subscriber_ptr)
	{
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return;

		it->second->second->NotifyNewShaderObjectData(shader, data, index_subscriber_ptr);
	}

	void RenderWorld::NotifyUpdateShaderObjectData(const Shader *shader,
												   Data data,
												   std::uint32_t index,
												   const hrs::block<vk::DeviceSize> &data_block,
												   vk::DeviceSize in_data_buffer_offset)
	{
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return;

		it->second->second->NotifyUpdateShaderObjectData(shader,
														 data,
														 index,
														 data_block,
														 in_data_buffer_offset);
	}

	void RenderWorld::NotifyRemoveShaderObjectData(const Shader *shader,
												   std::uint32_t index)
	{
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return;

		it->second->second->NotifyRemoveShaderObjectData(shader, index);
	}

	void RenderWorld::NotifyNewRenderGroupInstance(const RenderGroup *render_group,
												   std::uint32_t data_index,
												   std::uint32_t *subscriber_ptr)
	{
		const MaterialGroup *material_group = render_group->GetParentMaterialGroup();
		const Shader *shader = material_group->GetParentShader();
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return;

		it->second->second->NotifyNewRenderGroupInstance(render_group, data_index, subscriber_ptr);
	}

	void RenderWorld::NotifyRemoveRenderGroupInstance(const RenderGroup *render_group,
													  std::uint32_t index)
	{
		const MaterialGroup *material_group = render_group->GetParentMaterialGroup();
		const Shader *shader = material_group->GetParentShader();
		const RenderPass *renderpass = shader->GetParentRenderPass();
		auto it = renderpasses_search.find(renderpass);
		if(it == renderpasses_search.end())
			return;

		it->second->second->NotifyRemoveRenderGroupInstance(render_group, index);
	}

	hrs::error RenderWorld::Flush(std::uint32_t frame_index)
	{
		for(auto &renderpass : renderpasses)
		{
			auto err = renderpass.second->Flush(frame_index);
			if(err)
				return err;
		}

		return {};
	}

	hrs::expected<std::span<const vk::CommandBuffer>, vk::Result>
	RenderWorld::Render(std::uint32_t frame_index, vk::DescriptorSet globals_set) const noexcept
	{
		std::size_t fillness = 0;
		for(auto &rpass : renderpasses)
		{
			if(!rpass.second->GetState())
				continue;

			auto [command_buffer, result] = rpass.second->Render(frame_index, globals_set);
			if(result != vk::Result::eSuccess)
				return result;

			if(command_buffer)
				render_results[frame_index][fillness] = command_buffer;

			fillness++;
		}

		return std::span{render_results[frame_index].data(), fillness};
	}

	bool RenderWorld::HasRenderPass(const RenderPass *renderpass) const noexcept
	{
		auto it = renderpasses_search.find(renderpass);
		return it != renderpasses_search.end();
	}

	void RenderWorld::RemoveRenderPass(const RenderPass *rpass) noexcept
	{
		auto it = renderpasses_search.find(rpass);
		if(it == renderpasses_search.end())
			return;

		renderpasses_search.erase(it);
		renderpasses.erase(it->second);
	}

	const std::function<NewPoolSizeCalculator> & RenderWorld::GetNewPoolSizeCalculator() const noexcept
	{
		return calc;
	}

	Device * RenderWorld::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const Device * RenderWorld::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	std::uint32_t RenderWorld::GetFrameCount() const noexcept
	{
		return frame_count;
	}
};
