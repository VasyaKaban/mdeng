#pragma once

#include "../../../Context/Device.h"
#include "../../../../hrs/non_creatable.hpp"
#include "../../../../hrs/expected.hpp"
#include "../../../../hrs/flags.hpp"
#include "../../../../hrs/unexpected_result.hpp"

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
		constexpr static vk::ImageType IMAGE_TYPE = vk::ImageType::e2D;
		constexpr static vk::ImageTiling IMAGE_TILING = vk::ImageTiling::eOptimal;
		constexpr static vk::SampleCountFlagBits SAMPLES = vk::SampleCountFlagBits::e1;

	public:
		enum BufferIndices
		{
			ColorBuffer = 0,
			DepthStencilBuffer,
			LastUnusedBufferIndex
		};

		using BuffersArray = std::array<ImageFormatBounds, BufferIndices::LastUnusedBufferIndex>;

	private:

		void init(BuffersArray &&_buffers, const vk::Extent2D &_resolution) noexcept;

	public:

		GBuffer(Device *_parent_device) noexcept;

		hrs::unexpected_result Recreate(const GBufferImageParams &color_buffer_params,
										const GBufferImageParams &depth_buffer_params,
										const vk::Extent2D &_resolution);

		~GBuffer();
		GBuffer(GBuffer &&gbuf) noexcept;
		GBuffer & operator=(GBuffer &&gbuf) noexcept;

		void Destroy();

		bool IsCreated() const noexcept;

		const BuffersArray & GetBuffers() const noexcept;
		const ImageFormatBounds & GetBuffer(BufferIndices index) const noexcept;

		vk::Extent2D GetResolution() const noexcept;

	private:

		hrs::expected<BoundedImage, hrs::unexpected_result> allocate_image(const vk::ImageCreateInfo &info);

		Device *parent_device;
		BuffersArray buffers;
		vk::Extent2D resolution;
	};
};
