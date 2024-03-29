#pragma once

#include "../../Context/Device.h"
#include "../../../Allocator/AllocatorTypes.hpp"
#include "../../../hrs/unexpected_result.hpp"

namespace FireLand
{
	class PostProcessImages : public hrs::non_copyable
	{
		void init(std::array<BoundedImage, 2> &&_images,
				  vk::Sampler _sampler,
				  vk::Format _format,
				  const vk::Extent2D &_resolution) noexcept;
	public:

		PostProcessImages(Device *_parent_device) noexcept;
		~PostProcessImages();
		PostProcessImages(PostProcessImages &&ppi) noexcept;
		PostProcessImages & operator=(PostProcessImages &&ppi) noexcept;

		hrs::unexpected_result Recreate(const vk::SamplerCreateInfo &sampler_info,
										vk::Format _format,
										vk::ImageUsageFlags usage,
										vk::ImageLayout initial_layout,
										const vk::Extent2D &_resolution);

		void Destroy();
		bool IsCreated() const noexcept;

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		const std::array<BoundedImage, 2> & GetBoundedImages() const noexcept;
		vk::Sampler GetSampler() const noexcept;
		vk::Format GetFormat() const noexcept;
		std::size_t GetTargetOutputImageIndex() const noexcept;
		vk::Image GetOutputImage() const noexcept;
		vk::Image GetInputImage() const noexcept;

		void SwitchImages() noexcept;
		void ResetTargetOutputImageIndex() noexcept;

	private:

		hrs::expected<std::array<BoundedImage, 2>, hrs::unexpected_result>
		allocate_images(vk::Format _format,
						vk::ImageUsageFlags usage,
						vk::ImageLayout initial_layout,
						const vk::Extent2D &_resolution);

	private:
		Device *parent_device = {};
		std::array<BoundedImage, 2> images;
		vk::Sampler sampler;
		vk::Format format;
		vk::Extent2D resolution;
		std::size_t target_output_image = {};
	};
};
