#pragma once

#include <cstddef>
#include <map>
#include <unordered_map>
#include <set>
#include "../../../hrs/non_creatable.hpp"
#include "../../../hrs/mem_req.hpp"
#include "../../../Vulkan/VulkanInclude.hpp"
#include "../../DataBuffer/DataBuffer.h"
#include "../../DataIndexStorage/DataIndexStorage.h"
#include "../../DescriptorStorage/DescriptorStorage.h"
#include "RenderGroup.h"
#include "Stateful.h"
#include "Material.h"

namespace FireLand
{
	class RenderPass;
	class MaterialGroup;
	class Mesh;

	struct DescriptorSetGroupKey
	{
		DescriptorSetGroup descriptor_set_group;
		mutable const Material *material_compare;
	};

	struct MaterialComparator
	{
		using is_transparent  = void;
		bool operator()(const DescriptorSetGroupKey &key, const Material *mtl2) const noexcept
		{
			return key.material_compare->CompareLess(mtl2);
		}
	};

	class Shader
		: public hrs::non_copyable,
		  public Stateful
	{
	public:
		Shader(RenderPass *_parent_renderpass,
			   std::vector<vk::CommandBuffer> &&_command_buffers,
			   DataBuffer &&_data_buffer,
			   DataIndexStorage &&_data_index_storage,
			   DescriptorStorage &&_descriptor_storage) noexcept;
		virtual ~Shader();
		Shader(Shader &&s) noexcept;
		Shader & operator=(Shader &&s) noexcept;

		RenderPass * GetParentRenderPass() noexcept;
		const RenderPass * GetParentRenderPass() const noexcept;

		MaterialGroup * FindMaterialGroup(const Material *mtl) noexcept;
		const MaterialGroup * FindMaterialGroup(const Material *mtl) const noexcept;
		void RemoveMaterial(const Material *mtl) noexcept;

		vk::Result RebindMaterial(const Material *mtl);
		vk::Result RebindMaterial(const MaterialGroup *mtl_group);

		void AddObjectData(Data data,
						   std::uint32_t *index_subscriber_ptr);

		void UpdateObjectData(Data data,
							  std::uint32_t index,
							  const hrs::block<vk::DeviceSize> &data_block,
							  vk::DeviceSize in_data_buffer_offset);

		void RemoveObjectData(std::uint32_t index);

		std::pair<vk::CommandBuffer, vk::Result>
		Render(std::uint32_t frame_index,
			   vk::DescriptorSet globals_set,
			   vk::DescriptorSet renderpass_descriptor_set,
			   const vk::CommandBufferInheritanceInfo &inheritance_info) const noexcept;

		hrs::error Flush(std::uint32_t frame_index);

		void NotifyNewRenderGroupInstance(const RenderGroup *render_group,
										  std::uint32_t data_index,
										  std::uint32_t *subscriber_ptr);

		void NotifyRemoveRenderGroupInstance(const RenderGroup *render_group,
											 std::uint32_t index);

		virtual hrs::mem_req<vk::DeviceSize> GetDataMemoryRequirements() const noexcept = 0;

	protected:

		virtual void SetPerCallData(vk::CommandBuffer command_buffer,
									vk::Buffer plain_data_buffer,
									vk::Buffer plain_index_buffer,
									vk::DescriptorSet globals_descriptor_set,
									vk::DescriptorSet renderpass_descriptor_set) const noexcept = 0;

		//renderpass set -> input attachments
		//shader set -> data buffer and data index buffer(inner)
		//material set -> textures
		virtual void Bind(vk::CommandBuffer command_buffer,
						  vk::DescriptorSet material_set,
						  const Material *target_meterial,
						  const Material *prev_material) const noexcept = 0;

		virtual hrs::error FlushInner(std::uint32_t frame_index) noexcept = 0;

		using MaterialGroupsContainer = std::vector<MaterialGroup>;
		using MaterialGroupBindingsContainer = std::map<DescriptorSetGroupKey,
														MaterialGroupsContainer,
														MaterialComparator>;
		struct MaterialSearch
		{
			MaterialGroupsContainer::iterator material_group_it;
			MaterialGroupBindingsContainer::iterator binding_it;
		};

		using MaterialGroupsSearchContainer = std::unordered_map<const Material *, MaterialSearch>;


		hrs::expected<std::pair<MaterialSearch, bool>, vk::Result>
		add_material(Material *material, bool _enabled);

	private:
		MaterialGroup unlink_material_group(MaterialGroupsSearchContainer::iterator it) noexcept;

		hrs::expected<MaterialGroupBindingsContainer::iterator, vk::Result>
		create_binding(const Material *mtl);

		friend class MaterialGroup;
		DataIndexStorage * material_group_get_data_index_storage() noexcept;
		const DataIndexStorage * material_group_get_data_index_storage() const noexcept;
	private:
		RenderPass *parent_renderpass;//CLEANUP!!!
		std::vector<vk::CommandBuffer> command_buffers;
		DataBuffer data_buffer;
		DataIndexStorage data_index_storage;
		DescriptorStorage descriptor_storage;

		MaterialGroupBindingsContainer material_group_binding;
		MaterialGroupsSearchContainer materials_search;
	};
};
