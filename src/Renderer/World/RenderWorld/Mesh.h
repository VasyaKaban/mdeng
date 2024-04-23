#pragma once

#include "../../Vulkan/VulkanInclude.hpp"
#include <cstdint>

namespace FireLand
{
	struct Mesh
	{
		virtual ~Mesh() {}
		virtual void Render(vk::CommandBuffer command_buffer) const noexcept = 0;
		virtual std::pair<vk::Buffer, vk::DeviceSize> GetVertexBuffer() const noexcept = 0;
		virtual std::pair<vk::Buffer, vk::DeviceSize> GetIndexBuffer() const noexcept = 0;
		virtual std::uint32_t GetCount() const noexcept = 0;
		virtual bool IsVertexBufferRebindNeeded(const Mesh *mesh) const noexcept = 0;
		virtual bool IsIndexBufferRebindNeeded(const Mesh *mesh) const noexcept = 0;
	};
};
