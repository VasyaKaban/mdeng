#include "ImageView.h"
#include "../Context/Device.h"
#include "Image.h"
#include "ImageStorage.h"

namespace FireLand
{
    ImageView::ImageView(Image* _parent_image,
                         vk::ImageView _image_view,
                         const vk::ImageViewCreateInfo& _create_info) noexcept
        : parent_image(_parent_image),
          image_view(_image_view),
          create_info(_create_info)
    {}

    hrs::expected<ImageView, vk::Result>
    ImageView::Create(Image* _parent_image, const vk::ImageViewCreateInfo& _create_info) noexcept
    {
        if(_parent_image)
            return ImageView(_parent_image, VK_NULL_HANDLE, _create_info);

        vk::Device device_handle =
            _parent_image->GetParentStorage()->GetParentDevice()->GetHandle();
        auto [_image_view_res, _image_view] = device_handle.createImageView(_create_info);
        if(_image_view_res != vk::Result::eSuccess)
            return _image_view_res;

        return ImageView(_parent_image, _image_view, _create_info);
    }

    ImageView::~ImageView()
    {
        Destroy();
    }

    ImageView::ImageView(ImageView&& view) noexcept
        : parent_image(view.parent_image),
          image_view(std::exchange(view.image_view, VK_NULL_HANDLE)),
          create_info(view.create_info)
    {}

    ImageView& ImageView::operator=(ImageView&& view) noexcept
    {
        Destroy();
        parent_image = view.parent_image;
        image_view = std::exchange(view.image_view, VK_NULL_HANDLE);
        create_info = view.create_info;

        return *this;
    }

    void ImageView::Destroy() noexcept
    {
        if(!IsCreated())
            return;

        vk::Device device_handle = parent_image->GetParentStorage()->GetParentDevice()->GetHandle();
        device_handle.destroy(image_view);
        image_view = VK_NULL_HANDLE;
    }

    bool ImageView::IsCreated() const noexcept
    {
        return image_view;
    }

    Image* ImageView::GetParentImage() noexcept
    {
        return parent_image;
    }

    const Image* ImageView::GetParentImage() const noexcept
    {
        return parent_image;
    }

    vk::ImageView ImageView::GetHandle() const noexcept
    {
        return image_view;
    }

    const vk::ImageViewCreateInfo& ImageView::GetCreateInfo() const noexcept
    {
        return create_info;
    }

    vk::ImageView ImageView::Release() noexcept
    {
        return std::exchange(image_view, VK_NULL_HANDLE);
    }
};
