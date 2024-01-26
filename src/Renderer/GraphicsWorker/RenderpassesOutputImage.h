#pragma once

#include "../../hrs/non_creatable.hpp"
#include "../../hrs/expected.hpp"
#include "../../hrs/unexpected_result.hpp"
#include "../../Allocator/Allocator.hpp"

namespace FireLand
{
	class RenderpassesOutputImage : public hrs::non_copyable
	{
		RenderpassesOutputImage(Allocator *_allocator,
								BoundedImage &&_bounded_image,
								vk::Format _format) noexcept;
	public:

		static hrs::expected<RenderpassesOutputImage, hrs::unexpected_result>
		Create(Allocator *_allocator,
			   vk::PhysicalDevice ph_device,
			   vk::Format swapchain_format,
			   const vk::Extent2D &resolution);

		~RenderpassesOutputImage();
		RenderpassesOutputImage(RenderpassesOutputImage &&out_img) noexcept;
		RenderpassesOutputImage & operator=(RenderpassesOutputImage &&out_img) noexcept;

		void Destroy();

		bool IsCreated() const noexcept;

		const BoundedImage & GetBoundedImage() const noexcept;
		vk::Format GetFormat() const noexcept;

	private:
		Allocator *allocator;
		BoundedImage bounded_image;
		vk::Format format;
	};
};
