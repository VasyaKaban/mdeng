#pragma once

#include "../Allocator/AllocateFromMany.hpp"
#include "../Vulkan/VulkanInclude.hpp"
#include "hrs/expected.hpp"
#include "hrs/non_creatable.hpp"
#include <filesystem>
#include <map>
#include <unordered_map>

namespace FireLand
{
    struct ImageGroupKey
    {
        std::string name;
        std::function<NewPoolSizeCalculator> calc;
        std::vector<MemoryPropertyOpFlags> allocation_variants;

        ImageGroupKey(std::string_view _name,
                      const std::function<NewPoolSizeCalculator>& _calc,
                      const std::vector<MemoryPropertyOpFlags>& _allocation_variants)
            : name(_name),
              calc(_calc),
              allocation_variants(_allocation_variants)
        {}

        ImageGroupKey(std::string_view _name,
                      const std::function<NewPoolSizeCalculator>& _calc,
                      std::vector<MemoryPropertyOpFlags>&& _allocation_variants)
            : name(_name),
              calc(_calc),
              allocation_variants(std::move(_allocation_variants))
        {}
    };
};

template<>
struct std::hash<FireLand::ImageGroupKey>
{
    std::size_t operator()(const FireLand::ImageGroupKey& igk) const noexcept
    {
        return std::hash<std::string>{}(igk.name);
    }
};

namespace FireLand
{
    class Image;
    class Device;

    struct ImageStorageStringHash
    {
        using is_transparent = void;

        std::size_t operator()(const char* str) const noexcept
        {
            return std::hash<std::string_view>{}(str);
        }

        std::size_t operator()(const std::string& str) const noexcept
        {
            return std::hash<std::string_view>{}(str);
        }

        std::size_t operator()(const std::string_view& str) const noexcept
        {
            return std::hash<std::string_view>{}(str);
        }

        std::size_t operator()(std::size_t hash) const noexcept
        {
            return hash;
        }
    };

    struct ImageStoragePathComparator
    {
        using is_transparent = void;

        std::size_t operator()(const std::filesystem::path& path, const char* str) const noexcept
        {
            return path < str;
        }

        std::size_t operator()(const char* str, const std::filesystem::path& path) const noexcept
        {
            return str < path;
        }

        std::size_t operator()(const std::filesystem::path& path,
                               const std::string& str) const noexcept
        {
            return path < str;
        }

        std::size_t operator()(const std::string& str,
                               const std::filesystem::path& path) const noexcept
        {
            return str < path;
        }

        std::size_t operator()(const std::filesystem::path& path,
                               std::string_view str) const noexcept
        {
            return path < str;
        }

        std::size_t operator()(std::string_view str,
                               const std::filesystem::path& path) const noexcept
        {
            return str < path;
        }
    };

    class ImageStorage : public hrs::non_copyable
    {
    public:
        using ImagesMap = std::map<std::filesystem::path, Image, ImageStoragePathComparator>;
        using ImageGroups = std::unordered_map<ImageGroupKey, ImagesMap, ImageStorageStringHash>;

        ImageStorage(Device* _parent_device, std::initializer_list<ImageGroupKey> groups);
        ~ImageStorage();
        ImageStorage(ImageStorage&& image_storage) noexcept;
        ImageStorage& operator=(ImageStorage&& image_storage) noexcept;

        void Destroy();

        ImageGroups::iterator AddGroup(const ImageGroupKey& group);
        ImageGroups::iterator AddGroup(ImageGroupKey&& group);

        Device* GetParentDevice() noexcept;
        const Device* GetParentDevice() const noexcept;
        const ImageGroups& GetGroups() const noexcept;
        std::optional<ImageGroups::iterator> GetGroup(std::string_view name);
        std::optional<ImageGroups::const_iterator> GetGroup(std::string_view name) const;
        std::optional<ImageGroups::iterator> GetGroup(std::size_t hash);
        std::optional<ImageGroups::const_iterator> GetGroup(std::size_t hash) const;

        void RemoveGroup(std::string_view name);
        void RemoveGroup(std::size_t hash);
        void RemoveGroup(ImageGroups::const_iterator it);

        hrs::expected<ImagesMap::iterator, hrs::error> AddImage(ImageGroups::iterator group_it,
                                                                const std::filesystem::path& path,
                                                                const vk::ImageCreateInfo& info);

        std::optional<ImagesMap::iterator> GetImage(std::string_view group_name,
                                                    const std::filesystem::path& path) noexcept;
        std::optional<ImagesMap::iterator> GetImage(std::size_t group_hash,
                                                    const std::filesystem::path& path) noexcept;
        std::optional<ImagesMap::const_iterator>
        GetImage(std::string_view group_name, const std::filesystem::path& path) const noexcept;
        std::optional<ImagesMap::const_iterator>
        GetImage(std::size_t group_hash, const std::filesystem::path& path) const noexcept;
    private:
        Device* parent_device;
        ImageGroups image_groups;
    };
};
