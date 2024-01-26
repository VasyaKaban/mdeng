#include "GBuffer.h"
#include "../../../../hrs/debug.hpp"
#include "../../../../hrs/scoped_call.hpp"

namespace FireLand
{
	GBuffer::GBuffer(Device *_parent_device,
					 BuffersArray &&_buffers,
					 vk::Extent2D _resolution) noexcept
		: parent_device(_parent_device),
		  buffers(std::move(_buffers)),
		  resolution(_resolution) {}

	hrs::expected<GBuffer, AllocationError> GBuffer::Create(Device *_parent_device,
															const GBufferImageParams &color_buffer_params,
															const GBufferImageParams &depth_buffer_params,
															vk::Extent2D _resolution)
	{
		hrs::assert_true_debug(_parent_device, "Parent device is points to null!");
		hrs::assert_true_debug(_parent_device->GetDevice(), "Parent device isn't created yet!");

		static vk::ImageCreateInfo info({},
										vk::ImageType::e2D,
										{},
										{},
										1,
										1,
										vk::SampleCountFlagBits::e1,
										vk::ImageTiling::eOptimal,
										{},
										vk::SharingMode::eExclusive,
										{},
										{});

		info.extent = vk::Extent3D(_resolution, 1);
		Allocator *allocator = _parent_device->GetAllocator();

		BuffersArray _buffers;
		hrs::scoped_call buffers_dtor([&_buffers, _parent_device]()
		{
			for(auto &buffer : _buffers)
				_parent_device->GetAllocator()->Destroy(buffer.image, false);
		});


		auto color_buffer_exp = allocate_image(allocator, color_buffer_params.FillInfo(info));
		if(!color_buffer_exp)
			return color_buffer_exp.error();

		_buffers[BufferIndices::ColorBuffer] = std::move(color_buffer_exp.value());

		auto depth_buffer_exp = allocate_image(allocator, depth_buffer_params.FillInfo(info));
		if(!depth_buffer_exp)
			return depth_buffer_exp.error();

		_buffers[BufferIndices::DepthStencilBuffer] = std::move(color_buffer_exp.value());

		buffers_dtor.Drop();

		return GBuffer(_parent_device,
					   std::move(_buffers),
					   _resolution);
	}

	GBuffer GBuffer::CreateNull(Device *_parent_device) noexcept
	{
		hrs::assert_true_debug(_parent_device, "Parent device is points to null!");
		hrs::assert_true_debug(_parent_device->GetDevice(), "Parent device isn't created yet!");

		return GBuffer(_parent_device, {}, {});
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

	hrs::expected<BoundedImage, AllocationError>
	GBuffer::allocate_image(Allocator *allocator, const vk::ImageCreateInfo &info)
	{
		constexpr static std::pair<vk::MemoryPropertyFlagBits, MemoryTypeSatisfyOperation> pairs[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Only},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Any},
			{vk::MemoryPropertyFlagBits::eHostVisible, MemoryTypeSatisfyOperation::Any}
		};

		hrs::expected<BoundedImage, AllocationError> buffer_exp;
		for(const auto &pair : pairs)
		{
			buffer_exp = allocator->Create(info,
										   pair.first,
										   {},
										   pair.second);

			if(buffer_exp)
				return buffer_exp;
			else if(buffer_exp.error().keeps<vk::Result>())
				return buffer_exp;
		}

		return buffer_exp;
	}
};

