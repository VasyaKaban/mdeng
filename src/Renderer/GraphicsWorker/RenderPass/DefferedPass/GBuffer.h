#pragma once

#include "../../../Context/Device.h"
#include "../../../../hrs/non_creatable.hpp"
#include "../../../../hrs/expected.hpp"
#include "../../../../hrs/flags.hpp"

namespace FireLand
{
	struct ImageFormatBounds
	{
		BoundedImage image;
		vk::Format format;

		ImageFormatBounds(BoundedImage _image = {}, vk::Format _format = {})
			: image(_image), format(_format) {}

		ImageFormatBounds(const ImageFormatBounds &) = default;
		ImageFormatBounds(ImageFormatBounds &&img_f) noexcept
			: image(img_f.image), format(img_f.format)
		{
			img_f.image = {};
		}

		ImageFormatBounds & operator=(const ImageFormatBounds &) = default;
		ImageFormatBounds & operator=(ImageFormatBounds &&img_f) noexcept
		{
			image = img_f.image;
			format = img_f.format;

			img_f.image = {};
			return *this;
		}
	};

	struct GBufferImageParams
	{
		vk::ImageUsageFlags usage;
		vk::Format format;
		vk::ImageLayout initial_layout;

		constexpr GBufferImageParams(vk::ImageUsageFlags _usage = {},
									 vk::Format _format = {},
									 vk::ImageLayout _initial_layout = {})
			: usage(_usage), format(_format), initial_layout(_initial_layout) {}

		constexpr vk::ImageCreateInfo & FillInfo(vk::ImageCreateInfo &info) const noexcept
		{
			info.usage = usage;
			info.format = format;
			info.initialLayout = initial_layout;
			return info;
		}
	};

	class GBuffer : public hrs::non_copyable
	{
	public:
		enum BufferIndices
		{
			ColorBuffer = 0,
			DepthStencilBuffer,
			LastUnusedBufferIndex
		};

		using BuffersArray = std::array<ImageFormatBounds, BufferIndices::LastUnusedBufferIndex>;

	private:

		GBuffer(Device *_parent_device,
				BuffersArray &&_buffers,
				vk::Extent2D _resolution) noexcept;

	public:

		static hrs::expected<GBuffer, AllocationError> Create(Device *_parent_device,
															  const GBufferImageParams &color_buffer_params,
															  const GBufferImageParams &depth_buffer_params,
															  vk::Extent2D _resolution);

		static GBuffer CreateNull(Device *_parent_device) noexcept;


		~GBuffer();
		GBuffer(GBuffer &&gbuf) noexcept;
		GBuffer & operator=(GBuffer &&gbuf) noexcept;

		void Destroy();

		bool IsCreated() const noexcept;

		const BuffersArray & GetBuffers() const noexcept;
		const ImageFormatBounds & GetBuffer(BufferIndices index) const noexcept;

		vk::Extent2D GetResolution() const noexcept;

	private:

		static hrs::expected<BoundedImage, AllocationError>
		allocate_image(Allocator *allocator, const vk::ImageCreateInfo &info);

		Device *parent_device;
		BuffersArray buffers;
		vk::Extent2D resolution;
	};
};
