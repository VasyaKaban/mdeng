#include "GBuffer.h"
#include "../../../hrs/scoped_call.hpp"
#include "../../../Vulkan/UnexpectedVkResult.hpp"
#include "../../../Allocator/UnexpectedAllocationResult.hpp"
#include "../../../Vulkan/VulkanUtils.hpp"

namespace FireLand
{
	void GBuffer::init( BufferArray<ImageFormatBounds> &&_buffers, const vk::Extent2D &_resolution) noexcept
	{
		buffers = std::move(_buffers);
		resolution = _resolution;
	}

	GBuffer::GBuffer(Device *_parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device is points to null!");
		hrs::assert_true_debug(_parent_device->GetHandle(), "Parent device isn't created yet!");
	}

	hrs::unexpected_result
	GBuffer::Recreate(const BufferArray<GBufferImageParams> &buffers_params,
					  const vk::Extent2D &_resolution)
	{
		Destroy();

		if(IsBadExtent(_resolution))
		{
			resolution = vk::Extent2D{};
			return {};
		}

		vk::ImageCreateInfo info({},
								 vk::ImageType::e2D,
								 {/*format*/},
								 vk::Extent3D{_resolution, 1},
								 1,
								 1,
								 vk::SampleCountFlagBits::e1,
								 vk::ImageTiling::eOptimal,
								 {/*usage*/},
								 vk::SharingMode::eExclusive,
								 {},
								 {/*initial layout*/});

		BufferArray<vk::ImageCreateInfo> infos;
		for(std::size_t i = 0; i < BufferIndices::LastUnusedBufferIndex; i++)
			infos[i] = buffers_params[i].FillInfo(info);

		auto allocated_buffers_exp = allocate_buffers(infos);
		if(!allocated_buffers_exp)
			return allocated_buffers_exp.error();

		init(std::move(allocated_buffers_exp.value()), _resolution);
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

		Allocator *allocator = parent_device->GetAllocator();
		for(auto &buffer : buffers)
		{
			allocator->Destroy(buffer.bounded_image, false);
			buffer.bounded_image = {};
		}
	}

	bool GBuffer::IsCreated() const noexcept
	{
		return buffers[0].bounded_image.IsCreated();
	}

	const GBuffer::BufferArray<ImageFormatBounds> & GBuffer::GetBuffers() const noexcept
	{
		return buffers;
	}

	const ImageFormatBounds & GBuffer::GetBuffer(BufferIndices index) const noexcept
	{
		return buffers[index];
	}

	vk::Image GBuffer::GetBufferImage(BufferIndices index) const noexcept
	{
		return buffers[index].bounded_image.image;
	}

	vk::Extent2D GBuffer::GetResolution() const noexcept
	{
		return resolution;
	}

	Device * GBuffer::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const Device * GBuffer::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	hrs::expected<GBuffer::BufferArray<ImageFormatBounds>, hrs::unexpected_result>
	GBuffer::allocate_buffers(const BufferArray<vk::ImageCreateInfo> &infos)
	{
		constexpr static std::pair<vk::MemoryPropertyFlagBits, MemoryTypeSatisfyOperation> pairs[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Only},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Any},
			{vk::MemoryPropertyFlagBits{}, MemoryTypeSatisfyOperation::Any}
		};

		Allocator *allocator = parent_device->GetAllocator();
		BufferArray<ImageFormatBounds> out_buffers;
		hrs::scoped_call out_buffers_dtor([&out_buffers, allocator]()
		{
			for(const auto &buffer : out_buffers)
			{
				if(buffer.bounded_image.IsCreated())
					allocator->Destroy(buffer.bounded_image, false);
				else
					break;
			}
		});

		hrs::unexpected_result unexp_res;
		for(std::size_t i = 0; i < infos.size(); i++)
		{
			for(const auto &pair : pairs)
			{
				auto buffer_exp = allocator->Create(infos[i],
													pair.first,
													{},
													pair.second);

				if(!buffer_exp)
				{
					if(buffer_exp.error().keeps<vk::Result>())
						return {UnexpectedVkResult(buffer_exp.error().get<vk::Result>())};
					else
						unexp_res = UnexpectedAllocationResult(buffer_exp.error().get<AllocationResult>());
				}
				else
				{
					out_buffers[i] = buffer_exp.value();
					break;
				}
			}

			if(!out_buffers[i].bounded_image.IsCreated())
				return unexp_res;
		}

		out_buffers_dtor.Drop();
		return out_buffers;
	}
};

