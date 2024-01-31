#include "GBuffer.h"
#include "../../../../Vulkan/VulkanUtils.hpp"
#include "../../../../hrs/debug.hpp"
#include "../../../../hrs/scoped_call.hpp"
#include "../../../../Vulkan/UnexpectedVkResult.hpp"
#include "../../../../Allocator/UnexpectedAllocationResult.hpp"

namespace FireLand
{
	void GBuffer::init(BuffersArray &&_buffers, const vk::Extent2D &_resolution) noexcept
	{
		buffers = std::move(_buffers);
		resolution = _resolution;
	}

	GBuffer::GBuffer(Device *_parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device is points to null!");
		hrs::assert_true_debug(_parent_device->GetDevice(), "Parent device isn't created yet!");
	}

	hrs::unexpected_result
	GBuffer::Recreate(const GBufferImageParams &color_buffer_params,
					  const GBufferImageParams &depth_buffer_params,
					  const vk::Extent2D &_resolution)
	{
		if(IsBadExtent(_resolution))
			return {};

		static vk::ImageCreateInfo info({},
										IMAGE_TYPE,
										{},
										{},
										1,
										1,
										SAMPLES,
										IMAGE_TILING,
										{},
										vk::SharingMode::eExclusive,
										{},
										{});

		info.extent = vk::Extent3D(_resolution, 1);

		BuffersArray _buffers;
		hrs::scoped_call buffers_dtor([&_buffers, this]()
		{
			for(auto &buffer : _buffers)
				parent_device->GetAllocator()->Destroy(buffer.image, false);
		});


		auto color_buffer_exp = allocate_image(color_buffer_params.FillInfo(info));
		if(!color_buffer_exp)
			return std::move(color_buffer_exp.error());

		_buffers[BufferIndices::ColorBuffer].image = std::move(color_buffer_exp.value());
		_buffers[BufferIndices::ColorBuffer].format = color_buffer_params.format;

		auto depth_buffer_exp = allocate_image(depth_buffer_params.FillInfo(info));
		if(!depth_buffer_exp)
			return std::move(depth_buffer_exp.error());

		_buffers[BufferIndices::DepthStencilBuffer].image = std::move(depth_buffer_exp.value());
		_buffers[BufferIndices::DepthStencilBuffer].format = depth_buffer_params.format;

		buffers_dtor.Drop();

		init(std::move(_buffers), _resolution);
		return {};
	}

	GBuffer::~GBuffer()
	{
		Destroy();
	}

	GBuffer::GBuffer(GBuffer &&gbuf) noexcept
		: parent_device(gbuf.parent_device),
		  buffers(std::move(gbuf.buffers)),
		  resolution(gbuf.resolution) {}

	GBuffer & GBuffer::operator=(GBuffer &&gbuf) noexcept
	{
		Destroy();

		parent_device = gbuf.parent_device;
		buffers = std::move(gbuf.buffers);
		resolution = gbuf.resolution;

		return *this;
	}

	void GBuffer::Destroy()
	{
		if(!IsCreated())
			return;

		for(auto &buffer : buffers)
		{
			parent_device->GetAllocator()->Destroy(buffer.image, false);
			buffer.image = {};
		}
	}

	bool GBuffer::IsCreated() const noexcept
	{
		return buffers[0].image.IsCreated();
	}

	const GBuffer::BuffersArray & GBuffer::GetBuffers() const noexcept
	{
		return buffers;
	}

	const ImageFormatBounds & GBuffer::GetBuffer(BufferIndices index) const noexcept
	{
		return buffers[index];
	}

	vk::Extent2D GBuffer::GetResolution() const noexcept
	{
		return resolution;
	}

	hrs::expected<BoundedImage, hrs::unexpected_result>
	GBuffer::allocate_image(const vk::ImageCreateInfo &info)
	{
		constexpr static std::pair<vk::MemoryPropertyFlagBits, MemoryTypeSatisfyOperation> pairs[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Only},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Any},
			{vk::MemoryPropertyFlagBits::eHostVisible, MemoryTypeSatisfyOperation::Any}
		};

		Allocator *allocator = parent_device->GetAllocator();

		hrs::unexpected_result unexp_res;
		for(const auto &pair : pairs)
		{
			auto buffer_exp = allocator->Create(info,
												pair.first,
												{},
												pair.second);

			if(buffer_exp)
				return buffer_exp.value();
			else if(buffer_exp.error().keeps<vk::Result>())
				return {UnexpectedVkResult(buffer_exp.error().get<vk::Result>())};
			else
				unexp_res = UnexpectedAllocationResult(buffer_exp.error().get<AllocationResult>());
		}

		return unexp_res;
	}
};

