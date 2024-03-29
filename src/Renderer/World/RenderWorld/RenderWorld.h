#pragma once

#include <cstdint>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include "../../../hrs/block.hpp"
#include "../../../Vulkan/VulkanInclude.hpp"
#include "../../../hrs/error.hpp"
#include "../../../hrs/expected.hpp"
#include "RenderGroup.h"
#include "RenderPass.h"
#include "../../Allocator/MemoryType.h"
#include "../../TransferChannel/Data.h"
#include "../../Context/Device.h"

namespace FireLand
{
	class Mesh;
	class Material;
	class Shader;
	class RenderPass;

	/*struct RenderGroupOptions
	{
		Shader *shader;
		const Material *material;
		const Mesh *mesh;
		std::uint32_t init_size_power;
		std::uint32_t rounding_size;
	};*/

	class RenderWorld : public hrs::non_copyable
	{
	public:
		using RenderPassesContainer = std::multimap<std::size_t, std::unique_ptr<RenderPass>>;
		using RenderPassesSearchContainer =
			std::unordered_map<const RenderPass *, RenderPassesContainer::iterator>;

		RenderWorld(Device *_parent_device,
					std::uint32_t _frame_count,
					std::uint32_t _queue_family_index,
					const std::function<NewPoolSizeCalculator> &_calc) noexcept;
		~RenderWorld();
		RenderWorld(RenderWorld &&rw) noexcept;
		RenderWorld & operator=(RenderWorld &&rw) noexcept;


		/*RenderGroup * GetRenderGroup(const Material *material, const Mesh *mesh) const noexcept;

		RenderGroup * NotifyNewRenderGroup(const Material *material,
										   const Mesh *mesh,
										   const RenderGroupOptions &options);
		*/
		//void NotifyRemoveEmptyRenderGroup(RenderGroup *render_group);
		//void NotifyRemoveRenderGroup(RenderGroup *render_group);

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

		hrs::error Flush(std::uint32_t frame_index);

		hrs::expected<std::span<const vk::CommandBuffer>, vk::Result>
		Render(std::uint32_t frame_index,
			   vk::DescriptorSet globals_set) const noexcept;

		template<typename F>
		hrs::expected<RenderPass *, hrs::error>
		AddRenderPass(F &&rpass_creator, std::size_t priority);

		bool HasRenderPass(const RenderPass *renderpass) const noexcept;
		void RemoveRenderPass(const RenderPass *rpass) noexcept;

		const std::function<NewPoolSizeCalculator> & GetNewPoolSizeCalculator() const noexcept;
		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		std::uint32_t GetFrameCount() const noexcept;

	private:
		Device *parent_device;
		std::uint32_t frame_count;
		std::uint32_t queue_family_index;
		std::function<NewPoolSizeCalculator> calc;
		mutable std::vector<std::vector<vk::CommandBuffer>> render_results;
		RenderPassesContainer renderpasses;
		RenderPassesSearchContainer renderpasses_search;
		//renderpass -> shader -> material -> mesh -> render_group
	};

	template<typename F>
	hrs::expected<RenderPass *, hrs::error>
	RenderWorld::AddRenderPass(F &&rpass_creator, std::size_t priority)
	{
		vk::Device device_handle = parent_device->GetHandle();
		const vk::CommandPoolCreateInfo pool_info({}, queue_family_index);
		auto [u_pool_res, u_pool] = device_handle.createCommandPoolUnique(pool_info);
		if(u_pool_res != vk::Result::eSuccess)
			return u_pool_res;

		const vk::CommandBufferAllocateInfo command_buffer_info(u_pool.get(),
																vk::CommandBufferLevel::ePrimary,
																frame_count);

		auto [buffers_res, buffers] = device_handle.allocateCommandBuffers(command_buffer_info);
		if(buffers_res != vk::Result::eSuccess)
			return buffers_res;

		auto res = std::forward<F>(rpass_creator)(u_pool.get(), std::move(buffers));
		static_assert(hrs::instantiation<std::remove_cvref_t<decltype(res)>, hrs::expected>);
		if(!res)
		{
			device_handle.destroy(u_pool.release());
			return res.error();
		}

		auto it = renderpasses.insert({priority, res.value()});
		renderpasses_search.insert({res.value(), it});

		if(render_results.size() < frame_count)
			render_results.resize(frame_count);

		for(std::size_t i = 0; i < frame_count; i++)
			render_results[i].resize(renderpasses.size());

		return res.value();
	}
};
