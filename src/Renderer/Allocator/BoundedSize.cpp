#include "BoundedSize.h"

namespace FireLand
{
    BoundedBufferSize::BoundedBufferSize(BoundedBuffer&& bb, VkDeviceSize _size) noexcept
        : BoundedBuffer(std::move(bb)),
          size(_size)
    {}

    BoundedBufferSize::BoundedBufferSize(BoundedBufferSize&& buf) noexcept
        : BoundedBuffer(std::move(buf)),
          size(buf.size)
    {}

    BoundedBufferSize& BoundedBufferSize::operator=(BoundedBufferSize&& buf) noexcept
    {
        BoundedBuffer::operator=(std::move(buf));
        size = buf.size;

        return *this;
    }

    BoundedImageSize::BoundedImageSize(BoundedImage&& bi, VkDeviceSize _size) noexcept
        : BoundedImage(std::move(bi)),
          size(_size)
    {}

    BoundedImageSize::BoundedImageSize(BoundedImageSize&& img) noexcept
        : BoundedImage(std::move(img)),
          size(img.size)
    {}

    BoundedImageSize& BoundedImageSize::operator=(BoundedImageSize&& img) noexcept
    {
        BoundedImage::operator=(std::move(img));
        size = img.size;

        return *this;
    }

};
