#pragma once

#include "../../hrs/non_creatable.hpp"
#include "../../hrs/expected.hpp"
#include "../../hrs/unexpected_result.hpp"
#include "../../Allocator/Allocator.hpp"

namespace FireLand
{
	class RenderpassesOutputImage : public hrs::non_copyable
	{
		void init(BoundedImage &&_bounded_image, vk::Format _format) noexcept;

		constexpr static vk::ImageType IMAGE_TYPE = vk::ImageType::e2D;
		constexpr static vk::ImageTiling IMAGE_TILING = vk::ImageTiling::eOptimal;
		constexpr static std::uint32_t MIP_LEVELS = 1;
		constexpr static std::uint32_t ARRAY_LAYERS = 1;
		constexpr static vk::SampleCountFlagBits SAMPLES = vk::SampleCountFlagBits::e1;
		constexpr static vk::ImageCreateFlagBits CREATE_IMAGE_FLAGS = {};

	public:

		RenderpassesOutputImage(Allocator *_allocator) noexcept;

		hrs::unexpected_result Recreate(vk::PhysicalDevice ph_device,
										vk::Format swapchain_format,
										vk::FormatFeatureFlags format_features,
										vk::ImageUsageFlags image_usage,
										const vk::Extent2D &resolution);

		~RenderpassesOutputImage();
		RenderpassesOutputImage(RenderpassesOutputImage &&out_img) noexcept;
		RenderpassesOutputImage & operator=(RenderpassesOutputImage &&out_img) noexcept;

		void Destroy();

		bool IsCreated() const noexcept;

		const BoundedImage & GetBoundedImage() const noexcept;
		vk::Format GetFormat() const noexcept;

	private:

		static hrs::expected<vk::Format, vk::Result>
		find_best_format(vk::PhysicalDevice ph_device,
						 vk::Format swapchain_format,
						 vk::FormatFeatureFlags format_features,
						 vk::ImageUsageFlags image_usage,
						 const vk::Extent2D &resolution) noexcept;

		Allocator *allocator;
		BoundedImage bounded_image;
		vk::Format format;
	};
};
