#pragma once

#include "DefferedPassShader.h"
#include "../../../Context/Device.h"
#include "../../Shader/ShaderInfoNode.hpp"
#include "../../RenderInputs.hpp"
#include "../../../../Vulkan/VulkanUtils.hpp"

/*
mesh -> binds whole vertex and index buffers
part -> draws based on mesh offset and part offset: firstVertex = mesh.vertex_offset + part.vertex_offset
*/

/*
Mesh buffers:
Add -> transfer only
Delete -> noop
Update -> transfer + memory barrier

*/

struct Mesh
{
	virtual vk::Buffer GetBuffer() const noexcept = 0;
	virtual vk::DeviceSize GetVertexBufferOffset() const noexcept = 0;
	virtual vk::DeviceSize GetIndexBufferOffset() const noexcept = 0;
	virtual void BindVertexIndexBuffer(vk::CommandBuffer command_buffer) const noexcept = 0;
};

struct Part
{
	virtual Mesh * GetMesh() const noexcept;
	virtual void Draw(vk::CommandBuffer command_buffer, std::uint32_t instances_count) const noexcept = 0;
};

struct Shader
{

};

struct MaterialCompare
{
	std::size_t data_size;
	std::byte *data_ptr;
};

struct Material
{
	virtual void Bind(vk::CommandBuffer command_buffer, std::uint32_t instance_index_offset_push_constant) const noexcept = 0;
	virtual Shader * GetShader() const noexcept = 0;
	virtual MaterialCompare GetMaterialCompare() const noexcept = 0;
};

struct MeshInstance
{

};

struct MeshInstancePartDesc
{
	virtual Part * GetPart() const noexcept = 0;
	virtual Material * GetMaterial() const noexcept = 0;
};

struct ShaderBinding
{
	std::size_t renderpass_index;
	std::size_t subpass_index;
};

struct MeshVertexColorNormal
{
	struct Part
	{
		vk::DeviceSize vertex_buffer_offset;
		vk::DeviceSize index_buffer_offset;
		vk::DeviceSize vertices_count;
	};

	class Shader//iface
	{
	public:
		ShaderBinding GetShaderBinding() const noexcept;
		void BindDescriptors(vk::CommandBuffer command_buffer,
							 vk::DescriptorSet globals_set,
							 vk::DescriptorSet renderpass_set,
							 const VkDescriptorBufferInfo &instance_index_buffer,
							 const VkDescriptorBufferInfo &instance_data_buffer,
							 std::uint32_t instance_index_offset_push_constant);
	};

	struct Material
	{
		//vk::Image normal_map;
		Shader *shader;
	};

	struct MeshInstancePartDesc
	{
		Part *part;
		Material *material;
	};

	struct MeshInstance
	{
		std::vector<MeshInstancePartDesc> part_descriptions;
	};

	vk::DeviceSize vertex_buffer_offset;
	vk::DeviceSize index_buffer_offset;
	vk::DeviceSize vertices_size;

	vk::DeviceSize GetVertexBufferOffset() const noexcept;
	vk::DeviceSize GetIndexBufferOffset() const noexcept;

	std::vector<Part> parts;
	std::vector<MeshInstance> mesh_instances;
};

struct InstancePartDescCount
{
	MeshInstancePartDesc *instance_part_desc;
	std::uint32_t instance_count;
	std::uint32_t instance_data_index_buffer_offset;

	void BindMaterial(vk::CommandBuffer command_buffer) const noexcept
	{
		instance_part_desc->GetMaterial()->Bind(command_buffer, instance_data_index_buffer_offset);
	}

	void BindMeshVertexIndexBuffers(vk::CommandBuffer command_buffer) const noexcept
	{
		instance_part_desc->GetPart()->GetMesh()->BindVertexIndexBuffer(command_buffer);
	}

	void Draw(vk::CommandBuffer command_buffer) const noexcept
	{
		instance_part_desc->GetPart()->Draw(command_buffer, instance_count);
	}

	bool IsNonZeroInstances() const noexcept
	{
		return instance_count != 0;
	}

	bool IsSameMaterial(const InstancePartDescCount *instance_part_desc_count) const noexcept
	{
		return instance_part_desc->GetMaterial() == instance_part_desc_count->instance_part_desc->GetMaterial();
	}

	bool IsSameMesh(const InstancePartDescCount *instance_part_desc_count) const noexcept
	{
		return instance_part_desc->GetPart()->GetMesh() ==
			   instance_part_desc_count->instance_part_desc->GetPart()->GetMesh();
	}

	bool IsSameMeshVertexIndexBuffers(const InstancePartDescCount *instance_part_desc_count) const noexcept
	{
		return instance_part_desc->GetPart()->GetMesh()->GetBuffer() ==
			   instance_part_desc_count->instance_part_desc->GetPart()->GetMesh()->GetBuffer();
	}

	static bool comparator(const InstancePartDescCount &ins1, const InstancePartDescCount &ins2) noexcept
	{
		const bool material_comp =
			ins1.instance_part_desc->GetMaterial() < ins2.instance_part_desc->GetMaterial();

		if(material_comp)
			return true;

		return ins1.instance_part_desc->GetPart()->GetMesh() < ins2.instance_part_desc->GetPart()->GetMesh();
	}
};

#include <set>

using instance_descriptions_set =
	std::multiset<InstancePartDescCount, decltype(InstancePartDescCount::comparator)*>;

auto foo()
{
	vk::CommandBuffer command_buffer = {};
	const InstancePartDescCount *binded_instance_part_desc_count = nullptr;
	instance_descriptions_set instance_part_description_counts(InstancePartDescCount::comparator);
	for(const auto &instance_part_desc_count : instance_part_description_counts)
	{
		if(!instance_part_desc_count.IsNonZeroInstances())
			continue;

		if(!binded_instance_part_desc_count)
		{
			instance_part_desc_count.BindMaterial(command_buffer);
			instance_part_desc_count.BindMeshVertexIndexBuffers(command_buffer);
		}
		else
		{
			if(!instance_part_desc_count.IsSameMaterial(binded_instance_part_desc_count))
				instance_part_desc_count.BindMaterial(command_buffer);

			if(!instance_part_desc_count.IsSameMeshVertexIndexBuffers(binded_instance_part_desc_count))
				instance_part_desc_count.BindMeshVertexIndexBuffers(command_buffer);
		}

		instance_part_desc_count.Draw(command_buffer);

		binded_instance_part_desc_count = &instance_part_desc_count;
	}
}

/*
for(auto &mesh : meshes)
{
	for(auto &instance : mesh.instances)
	{
		for(auto &part_desc : instance.part_descriptions)
		{
			part_desc->BindPart();
			part_desc->BindMaterial();
			part_desc->DrawPart();
		}
	}
}

 */

/*

struct InstancePartDescCount
{
	InstancePartDesc *instance_part_desc; -> part and material
	std::uint32_t instance_count;
	std::uint32_t instance_data_index_buffer_offset;

};


vk::CommandBuffer command_buffer;
const InstancePartDesc * binded_part_desc = nullptr;
std::set<InstancePartDescCount> instance_part_descriptions; -> set per renderpass!!!!!
for(auto &part_desc : instance_part_descriptions)
{
	if(!binded_part_desc)
	{
		part_desc.instance_part_desc->GetMaterial()->Bind(command_buffer, part_desc.instance_data_index_buffer_offset);
		part_desc.instance_part_desc->GetPart()->GetMesh()->Bind(command_buffer);
		part_desc.instance_part_desc->GetPart()->DrawPart(command_buffer, part_desc.instance_count);

		binded_part_desc = &part_desc.instance_part_desc;
	}
	else
	{
		if(binded_part_desc->GetMaterial() != part_desc.instance_part_desc->GetMaterial())
			part_desc.instance_part_desc->GetMaterial()->Bind(command_buffer, part_desc.instance_data_index_buffer_offset);

		if(binded_part_desc->GetPart()->GetMesh() != part_desc.instance_part_desc->GetPart()->GetMesh())
			part_desc.instance_part_desc->GetPart()->GetMesh()->Bind(command_buffer);

		part_desc.instance_part_desc->GetPart()->DrawPart(command_buffer, part_desc.instance_count);

		binded_part_desc = &part_desc.instance_part_desc;
	}
}
*/

/*



 */

/*


	template<typename M>
	struct Mesh
	{
		std::vector<M::Part> parts;
		std::vector<M::Instance> instances;
	}

	struct MeshPartInfo
	{
		MeshPart *part;
		Material *material;
		std::uint32_t instance_count;
	}

	struct RenderingNode
	{
		Shader *shader;
		std::vector<MeshPartInfo> mesh_parts;
	}

	assume that we have map of rendering nodes and key -> shader!

	for(auto &rendering_node : rendering_nodes
	{
		rendering_node.shader->Bind();
		for(auto &mesh_part : rendering_node.mesh_parts)
		{
			mesh_part.material->Bind();
			mesh_part.part->Bind();
			rendering_node.shader->Draw(mesh_part.instance_count);
		}
	}

	/////

	for(auto &mesh : meshes)
	{
		for(auto &rendering_node : mesh.rendering_nodes)
		{
			part.BindVertexBuffer();
			part.BindIndexBuffer();
			for(auto &shader : part.shaders)
			{
				shader.Bind();
				shader.Draw();
			}
		}
	}

*/

namespace FireLand
{
	class RenderingWorker;

	class DefferedPass : public hrs::non_copyable
	{
	public:
		enum AttachmentIndices
		{
			GBufferColor = 0,
			GBufferDepthStencil,
			GBufferNormals,
			GBufferImagesBorder,
			EvaluationImage = GBufferImagesBorder,
			LastUnusedAttachmentIndex
		};

		enum SubpassIndices
		{
			GeometrySubpassIndex = 0,
			LightSubpassIndex,
			LastUnusedSubpassIndex
		};

		using FramebufferImageViewsArray = std::array<vk::ImageView,
													  AttachmentIndices::LastUnusedAttachmentIndex>;

		using SubpassShaders = std::vector<ShaderInfoNode<DefferedPassShader>>;

		using ShaderSubpassBindingsArray =
			std::array<SubpassShaders, SubpassIndices::LastUnusedSubpassIndex>;

	private:

		void init(vk::RenderPass _renderpass,
				  vk::Framebuffer _framebuffer,
				  const FramebufferImageViewsArray &_framebuffer_image_views,
				  vk::DescriptorPool _descriptor_pool,
				  vk::DescriptorSetLayout _descriptor_set_layout,
				  std::vector<vk::DescriptorSet> &&_descriptor_sets,
				  const vk::Extent2D &_resolution) noexcept;

		void init(vk::RenderPass _renderpass,
				  vk::Framebuffer _framebuffer,
				  const FramebufferImageViewsArray &_framebuffer_image_views,
				  const vk::Extent2D &_resolution) noexcept;

	public:

		DefferedPass(RenderingWorker *_parent_worker) noexcept;

		~DefferedPass();
		DefferedPass(DefferedPass &&rpass) noexcept;
		DefferedPass & operator=(DefferedPass &&rpass) noexcept;

		void Destroy();

		hrs::unexpected_result Recreate(const vk::Extent2D &_resolution, std::uint32_t frame_count);

		void Render(vk::CommandBuffer command_buffer,
					std::uint32_t frame_index,
					const RenderInputs &inputs);

		bool IsCreated() const noexcept;

		hrs::unexpected_result AddShader(SubpassIndices subpass_index,
										 DefferedPassShader *shader,
										 const std::map<vk::ShaderStageFlagBits, ShaderInfo> &shader_infos);

		const ShaderSubpassBindingsArray & GetShaderSubpassBindingsArray() const noexcept;
		const SubpassShaders & GetSubpassShaders(SubpassIndices subpass_index) const noexcept;

		void DropShader(SubpassIndices subpass_index, SubpassShaders::const_iterator it) noexcept;
		void DropShader(SubpassIndices subpass_index, const DefferedPassShader *shader) noexcept;

	private:

		void destroy_renderpass_and_framebuffer() noexcept;
		void destroy_descriptor_sets() noexcept;
		void destroy_shader_bindings() noexcept;

		vk::ResultValue<vk::UniqueRenderPass> create_renderpass() noexcept;

		using create_framebuffer_result_t = std::tuple<vk::UniqueFramebuffer, FramebufferImageViewsArray>;

		hrs::expected<create_framebuffer_result_t, vk::Result>
		create_framebuffer(vk::RenderPass rpass, const vk::Extent2D &_resolution) noexcept;

		using create_descriptor_sets_result_t = std::tuple<vk::UniqueDescriptorPool,
														   vk::UniqueDescriptorSetLayout,
														   std::vector<vk::DescriptorSet>>;


		hrs::expected<create_descriptor_sets_result_t, vk::Result>
		create_descriptor_sets(const FramebufferImageViewsArray &views, std::uint32_t frame_count);

		void write_descriptor_sets(const FramebufferImageViewsArray &views,
								   const std::vector<vk::DescriptorSet> &sets) const noexcept;

	private:
		RenderingWorker *parent_worker;

		vk::RenderPass renderpass;
		vk::Framebuffer framebuffer;
		FramebufferImageViewsArray framebuffer_image_views;

		vk::DescriptorPool descriptor_pool;
		vk::DescriptorSetLayout descriptor_set_layout;
		std::vector<vk::DescriptorSet> descriptor_sets;

		ShaderSubpassBindingsArray shader_bindings;

		vk::Extent2D resolution;
	};
};
