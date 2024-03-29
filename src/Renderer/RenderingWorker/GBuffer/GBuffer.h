#pragma once

#include "../../Context/Device.h"
#include "../../../hrs/non_creatable.hpp"
#include "../../../hrs/expected.hpp"
#include "../../../hrs/unexpected_result.hpp"

namespace FireLand
{
	struct ImageFormatBounds
	{
		BoundedImage bounded_image;
		vk::Format format;

		ImageFormatBounds(BoundedImage _bounded_image = {}, vk::Format _format = {})
			: bounded_image(_bounded_image), format(_format) {}

		ImageFormatBounds(const ImageFormatBounds &) = default;
		ImageFormatBounds(ImageFormatBounds &&img_f) noexcept
			: bounded_image(img_f.bounded_image), format(img_f.format)
		{
			img_f.bounded_image = {};
		}

		ImageFormatBounds & operator=(const ImageFormatBounds &) = default;
		ImageFormatBounds & operator=(ImageFormatBounds &&img_f) noexcept
		{
			bounded_image = img_f.bounded_image;
			format = img_f.format;

			img_f.bounded_image = {};
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

		constexpr vk::ImageCreateInfo FillInfo(const vk::ImageCreateInfo &info) const noexcept
		{
			auto new_info = info;
			new_info.usage = usage;
			new_info.format = format;
			new_info.initialLayout = initial_layout;
			return new_info;
		}
	};

	class GBuffer : public hrs::non_copyable
	{
	public:
		enum BufferIndices
		{
			ColorBuffer = 0,
			DepthStencilBuffer,
			NormalsBuffer,
			LastUnusedBufferIndex
		};

		template<typename T>
		using BufferArray = std::array<T, BufferIndices::LastUnusedBufferIndex>;

	private:

		void init(BufferArray<ImageFormatBounds> &&_buffers, const vk::Extent2D &_resolution) noexcept;

	public:

		GBuffer(Device *_parent_device) noexcept;

		hrs::unexpected_result Recreate(const BufferArray<GBufferImageParams> &buffers_params,
										const vk::Extent2D &_resolution);

		~GBuffer();
		GBuffer(GBuffer &&gbuf) noexcept;
		GBuffer & operator=(GBuffer &&gbuf) noexcept;

		void Destroy();

		bool IsCreated() const noexcept;

		const BufferArray<ImageFormatBounds> & GetBuffers() const noexcept;
		const ImageFormatBounds & GetBuffer(BufferIndices index) const noexcept;
		vk::Image GetBufferImage(BufferIndices index) const noexcept;
		vk::Extent2D GetResolution() const noexcept;
		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;

	private:

		hrs::expected<BufferArray<ImageFormatBounds>, hrs::unexpected_result>
		allocate_buffers(const BufferArray<vk::ImageCreateInfo> &infos);

	private:
		Device *parent_device = {};
		BufferArray<ImageFormatBounds> buffers;
		vk::Extent2D resolution;
	};
};
