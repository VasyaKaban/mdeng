#include "ImageStorage.h"
#include "Image.h"

namespace FireLand
{
    ImageStorage::ImageStorage(Device* _parent_device, std::initializer_list<ImageGroupKey> groups)
        : parent_device(_parent_device)
    {
        image_groups.reserve(groups.size());
        for(auto& group: groups)
            image_groups.insert({group, {}});
    }

    ImageStorage::~ImageStorage()
    {
        Destroy();
    }

    ImageStorage::ImageStorage(ImageStorage&& image_storage) noexcept
        : parent_device(image_storage.parent_device),
          image_groups(std::move(image_storage.image_groups))
    {}

    ImageStorage& ImageStorage::operator=(ImageStorage&& image_storage) noexcept
    {
        Destroy();

        parent_device = image_storage.parent_device;
        image_groups = std::move(image_storage.image_groups);

        return *this;
    }

    void ImageStorage::Destroy()
    {
        image_groups.clear();
    }

    ImageStorage::ImageGroups::iterator ImageStorage::AddGroup(const ImageGroupKey& group)
    {
        return image_groups.emplace(group, ImagesMap{}).first;
    }

    ImageStorage::ImageGroups::iterator ImageStorage::AddGroup(ImageGroupKey&& group)
    {
        return image_groups.emplace(std::move(group), ImagesMap{}).first;
    }

    Device* ImageStorage::GetParentDevice() noexcept
    {
        return parent_device;
    }

    const Device* ImageStorage::GetParentDevice() const noexcept
    {
        return parent_device;
    }

    const ImageStorage::ImageGroups& ImageStorage::GetGroups() const noexcept
    {
        return image_groups;
    }

    std::optional<ImageStorage::ImageGroups::iterator> ImageStorage::GetGroup(std::string_view name)
    {
        auto it = image_groups.find(name);
        if(it == image_groups.end())
            return {};

        return it;
    }

    std::optional<ImageStorage::ImageGroups::const_iterator>
    ImageStorage::GetGroup(std::string_view name) const
    {
        auto it = image_groups.find(name);
        if(it == image_groups.end())
            return {};

        return it;
    }

    std::optional<ImageStorage::ImageGroups::iterator> ImageStorage::GetGroup(std::size_t hash)
    {
        auto it = image_groups.find(hash);
        if(it == image_groups.end())
            return {};

        return it;
    }

    std::optional<ImageStorage::ImageGroups::const_iterator>
    ImageStorage::GetGroup(std::size_t hash) const
    {
        auto it = image_groups.find(hash);
        if(it == image_groups.end())
            return {};

        return it;
    }

    void ImageStorage::RemoveGroup(std::string_view name)
    {
        auto it = image_groups.find(name);
        if(it != image_groups.end())
            image_groups.erase(it);
    }

    void ImageStorage::RemoveGroup(std::size_t hash)
    {
        auto it = image_groups.find(hash);
        if(it != image_groups.end())
            image_groups.erase(it);
    }

    void ImageStorage::RemoveGroup(ImageGroups::const_iterator it)
    {
        image_groups.erase(it);
    }

    hrs::expected<ImageStorage::ImagesMap::iterator, hrs::error>
    ImageStorage::AddImage(ImageGroups::iterator group_it,
                           const std::filesystem::path& path,
                           const vk::ImageCreateInfo& info)
    {
        auto image_it = group_it->second.find(path);
        if(image_it != group_it->second.end())
            return image_it;

        auto image_exp =
            Image::Create(this, group_it->first.allocation_variants, group_it->first.calc, info);
        if(!image_exp)
            return image_exp.error();

        return group_it->second.emplace(path, std::move(image_exp.value())).first;
    }

    std::optional<ImageStorage::ImagesMap::iterator>
    ImageStorage::GetImage(std::string_view group_name, const std::filesystem::path& path) noexcept
    {
        auto group_it = image_groups.find(group_name);
        if(group_it == image_groups.end())
            return {};

        auto image_it = group_it->second.find(path);
        if(image_it == group_it->second.end())
            return {};

        return image_it;
    }

    std::optional<ImageStorage::ImagesMap::iterator>
    ImageStorage::GetImage(std::size_t group_hash, const std::filesystem::path& path) noexcept
    {
        auto group_it = image_groups.find(group_hash);
        if(group_it == image_groups.end())
            return {};

        auto image_it = group_it->second.find(path);
        if(image_it == group_it->second.end())
            return {};

        return image_it;
    }

    std::optional<ImageStorage::ImagesMap::const_iterator>
    ImageStorage::GetImage(std::string_view group_name,
                           const std::filesystem::path& path) const noexcept
    {
        auto group_it = image_groups.find(group_name);
        if(group_it == image_groups.end())
            return {};

        auto image_it = group_it->second.find(path);
        if(image_it == group_it->second.end())
            return {};

        return image_it;
    }

    std::optional<ImageStorage::ImagesMap::const_iterator>
    ImageStorage::GetImage(std::size_t group_hash, const std::filesystem::path& path) const noexcept
    {
        auto group_it = image_groups.find(group_hash);
        if(group_it == image_groups.end())
            return {};

        auto image_it = group_it->second.find(path);
        if(image_it == group_it->second.end())
            return {};

        return image_it;
    }
};
