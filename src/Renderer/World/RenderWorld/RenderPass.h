#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>
#include <memory>
#include "hrs/error.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"
#include "../../Vulkan/VulkanInclude.hpp"
#include "Stateful.h"
#include "../../TransferChannel/Data.h"

namespace FireLand
{
	class RenderWorld;
	class Shader;
	class RenderGroup;
	class Material;
	class MaterialGroup;
	class Mesh;

	class RenderPass
		: public hrs::non_copyable,
		  public Stateful
	{
	public:
		RenderPass(RenderWorld *_parent_world,
				   std::vector<vk::CommandBuffer> &&_command_buffers,
				   vk::CommandPool _command_pool) noexcept;
		virtual ~RenderPass();
		RenderPass(RenderPass &&rpg) noexcept;
		RenderPass & operator=(RenderPass &&rpg) noexcept;

		std::optional<std::uint32_t> FindSubpass(const Shader *shader) const noexcept;
		void RemoveShader(const Shader * shader) noexcept;

		std::pair<vk::CommandBuffer, vk::Result>
		Render(std::uint32_t frame_index,
			   vk::DescriptorSet globals_set);

		hrs::error Flush(std::uint32_t frame_index);

		RenderWorld * GetParentWorld() noexcept;
		const RenderWorld * GetParentWorld() const noexcept;

		vk::Result RebindMaterial(const Shader *shader, const Material *mtl);
		vk::Result RebindMaterial(const MaterialGroup *mtl_group);

		void NotifyNewShaderObjectData(const Shader *shader,
									   Data data,
									   std::uint32_t *index_subscriber_ptr);

		void NotifyUpdateShaderObjectData(const Shader *shader,
										  Data data,
										  std::uint32_t index,
										  const hrs::block<vk::DeviceSize> &data_block,
										  vk::DeviceSize in_data_buffer_offset);

		void NotifyRemoveShaderObjectData(const Shader *shader,
										  std::uint32_t index);

		void NotifyNewRenderGroupInstance(const RenderGroup *render_group,
										  std::uint32_t data_index,
										  std::uint32_t *subscriber_ptr);

		void NotifyRemoveRenderGroupInstance(const RenderGroup *render_group,
											 std::uint32_t index);

		RenderGroup * AddRenderGroup(const Shader *shader,
									 const Material *mtl,
									 const Mesh *mesh,
									 std::uint32_t init_size_power,
									 std::uint32_t rounding_size,
									 bool _enabled);

		virtual vk::Extent2D GetResolution() const noexcept = 0;
		virtual std::uint32_t GetSubpassCount() const noexcept = 0;
		virtual vk::CommandBufferInheritanceInfo GetInheritanceInfo() const noexcept = 0;
		virtual vk::DescriptorSet GetDescriptorSet(std::uint32_t frame_index) const noexcept = 0;

		using SubpassShaderBinding = std::multimap<std::size_t, std::unique_ptr<Shader>>;
		struct ShaderSearch
		{
			SubpassShaderBinding::iterator it;
			std::uint32_t subpass;
		};
		using ShaderSearchContainer = std::unordered_map<const Shader *, ShaderSearch>;
	protected:
		virtual void Start(vk::CommandBuffer command_buffer) const noexcept = 0;
		virtual void NextSubpass(vk::CommandBuffer command_buffer) const noexcept = 0;
		virtual void End(vk::CommandBuffer command_buffer) const noexcept = 0;
		virtual hrs::error FlushInner(std::uint32_t frame_index) const noexcept = 0;
		virtual hrs::error Resize(const vk::Extent2D &resoulution) noexcept = 0;

		std::pair<SubpassShaderBinding::iterator, bool>
		add_shader(Shader *shader,
				   std::uint32_t subpass,
				   std::size_t priority);

		hrs::expected<std::vector<vk::CommandBuffer>, vk::Result>
		allocate_secondary_command_buffers() noexcept;

	private:
		void destroy() noexcept;

		friend class Shader;
		void shader_free_command_buffers(std::span<const vk::CommandBuffer> shader_buffers) noexcept;
	private:

		RenderWorld *parent_world;
		std::vector<vk::CommandBuffer> command_buffers;
		vk::CommandPool command_pool;
		std::vector<SubpassShaderBinding> subpass_shaders;
		ShaderSearchContainer shaders_search;
	};
};

