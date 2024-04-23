#pragma once

#include "../Vulkan/VulkanInclude.hpp"
#include "hrs/non_creatable.hpp"
#include "hrs/expected.hpp"

namespace FireLand
{
	class Image;

	class ImageView : public hrs::non_copyable
	{
	public:
		ImageView(Image *_parent_image = {},
				  vk::ImageView _image_view = {},
				  const vk::ImageViewCreateInfo &_create_info = {}) noexcept;

		static hrs::expected<ImageView, vk::Result>
		Create(Image *_parent_image,
			   const vk::ImageViewCreateInfo &_create_info) noexcept;

		~ImageView();
		ImageView(ImageView &&view) noexcept;
		ImageView & operator=(ImageView &&view) noexcept;

		void Destroy() noexcept;
		bool IsCreated() const noexcept;

		Image * GetParentImage() noexcept;
		const Image * GetParentImage() const noexcept;
		vk::ImageView GetHandle() const noexcept;
		const vk::ImageViewCreateInfo & GetCreateInfo() const noexcept;

		vk::ImageView Release() noexcept;

	private:
		Image *parent_image;
		vk::ImageView image_view;

		vk::ImageViewCreateInfo create_info;
	};
};
