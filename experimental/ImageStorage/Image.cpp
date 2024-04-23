#include "Image.h"
#include "ImageStorage.h"
#include "ImageView.h"
#include "../Context/Device.h"

namespace FireLand
{
	Image::Image(ImageStorage *_parent_storage,
				 BoundedImageSize &&_bounded_image_size,
				 const vk::ImageCreateInfo &_create_info) noexcept
		: parent_storage(_parent_storage),
		  bounded_image_size(std::move(_bounded_image_size)),
		  create_info(_create_info){}

	hrs::expected<Image, hrs::error>
	Image::Create(ImageStorage *_parent_storage,
				  std::span<const MemoryPropertyOpFlags> allocation_variants,
				  const std::function<NewPoolSizeCalculator> &calc,
				  const vk::ImageCreateInfo &_create_info) noexcept
	{
		if(!_parent_storage)
			return Image(_parent_storage, {}, _create_info);

		auto allocated_image_exp = AllocateFromMany(*_parent_storage->GetParentDevice()->GetAllocator(),
													allocation_variants,
													_create_info,
													1,
													calc);

		if(!allocated_image_exp)
			return allocated_image_exp.error();

		return Image(_parent_storage, std::move(allocated_image_exp.value()), _create_info);
	}

	Image::~Image()
	{
		Destroy();
	}

	Image::Image(Image &&image) noexcept
		: parent_storage(image.parent_storage),
		  bounded_image_size(std::move(image.bounded_image_size)),
		  create_info(image.create_info),
		  image_views(std::move(image.image_views)) {}

	Image & Image::operator=(Image &&image) noexcept
	{
		Destroy();

		parent_storage = image.parent_storage;
		bounded_image_size = std::move(image.bounded_image_size);
		create_info = image.create_info;
		image_views = std::move(image.image_views);

		return *this;
	}

	void Image::Destroy()
	{
		if(!IsCreated())
			return;

		parent_storage->GetParentDevice()->GetAllocator()->Release(bounded_image_size.bounded_image,
																   MemoryPoolOnEmptyPolicy::Free);

		bounded_image_size = {};
	}

	bool Image::IsCreated() const noexcept
	{
		return bounded_image_size.bounded_image.IsCreated();
	}

	ImageStorage * Image::GetParentStorage() noexcept
	{
		return parent_storage;
	}

	const ImageStorage * Image::GetParentStorage() const noexcept
	{
		return parent_storage;
	}

	const BoundedImageSize & Image::GetBoundedImageSize() const noexcept
	{
		return bounded_image_size;
	}

	const vk::ImageCreateInfo & Image::GetCreateInfo() const noexcept
	{
		return create_info;
	}

	std::list<ImageView> & Image::GetImageViews() noexcept
	{
		return image_views;
	}

	const std::list<ImageView> & Image::GetImageViews() const noexcept
	{
		return image_views;
	}

	hrs::expected<std::list<ImageView>::iterator, vk::Result>
	Image::CreateImageView(const vk::ImageViewCreateInfo &info)
	{
		auto image_view_exp = ImageView::Create(this, info);
		if(!image_view_exp)
			return image_view_exp.error();

		image_views.push_back(std::move(image_view_exp.value()));
		return std::prev(image_views.end());
	}

	vk::ImageView Image::ReleaseImageView(std::list<ImageView>::iterator it) noexcept
	{
		vk::ImageView view = it->Release();
		image_views.erase(it);
		return view;
	}
};
