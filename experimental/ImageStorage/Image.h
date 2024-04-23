#pragma once

#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include "../Allocator/BoundedResourceSize.hpp"
#include "../Allocator/AllocateFromMany.hpp"
#include <list>

namespace FireLand
{
	class ImageStorage;
	class ImageView;

	class Image : public hrs::non_copyable
	{
	public:
		Image(ImageStorage *_parent_storage = {},
			  BoundedImageSize &&_bounded_image_size = {},
			  const vk::ImageCreateInfo &_create_info = {}) noexcept;

		static hrs::expected<Image, hrs::error>
		Create(ImageStorage *_parent_storage,
			   std::span<const MemoryPropertyOpFlags> allocation_variants,
			   const std::function<NewPoolSizeCalculator> &calc,
			   const vk::ImageCreateInfo &_create_info) noexcept;

		~Image();
		Image(Image &&image) noexcept;
		Image & operator=(Image &&image) noexcept;

		void Destroy();
		bool IsCreated() const noexcept;

		ImageStorage * GetParentStorage() noexcept;
		const ImageStorage * GetParentStorage() const noexcept;
		const BoundedImageSize & GetBoundedImageSize() const noexcept;
		const vk::ImageCreateInfo & GetCreateInfo() const noexcept;
		std::list<ImageView> & GetImageViews() noexcept;
		const std::list<ImageView> & GetImageViews() const noexcept;

		hrs::expected<std::list<ImageView>::iterator, vk::Result>
		CreateImageView(const vk::ImageViewCreateInfo &info);

		vk::ImageView ReleaseImageView(std::list<ImageView>::iterator it) noexcept;

	private:
		ImageStorage *parent_storage;
		BoundedImageSize bounded_image_size;

		vk::ImageCreateInfo create_info;
		std::list<ImageView> image_views;
	};
};
