#pragma once

#include "../../DescriptorStorage/DescriptorStorage.h"
#include "../../Vulkan/VulkanInclude.hpp"
#include "PlainStateful.h"
#include "RenderGroup.h"
#include "hrs/hint_cast_object.hpp"
#include <map>
#include <unordered_map>

namespace FireLand
{
    class Mesh;
    class Shader;
    class Material;

    class MaterialGroup : public PlainStateful, public hrs::non_copyable
    {
    public:
        using RenderGroupsContainer = std::map<const Mesh*, RenderGroup>;
        using RenderGroupsSearchContainer =
            std::unordered_map<const RenderGroup*, RenderGroupsContainer::iterator>;

        MaterialGroup(Shader* _parent_shader, const Material* _material, bool _enabled) noexcept;
        ~MaterialGroup();
        MaterialGroup(MaterialGroup&& mtl) noexcept;
        MaterialGroup& operator=(MaterialGroup&& mtl) noexcept;

        RenderGroup& AddRenderGroup(const Mesh* mesh,
                                    std::uint32_t init_size_power,
                                    std::uint32_t rounding_size,
                                    bool _enabled);

        void RemoveMesh(const Mesh* mesh) noexcept;
        RenderGroup* FindRenderGroup(const Mesh* mesh) noexcept;
        const RenderGroup* FindRenderGroup(const Mesh* mesh) const noexcept;

        Shader* GetParentShader() noexcept;
        const Shader* GetParentShader() const noexcept;
        const Material* GetMaterial() const noexcept;

        void Flush();

        void NotifyNewRenderGroupInstance(const RenderGroup* render_group,
                                          std::uint32_t data_index,
                                          std::uint32_t* subscriber_ptr);

        void NotifyRemoveRenderGroupInstance(const RenderGroup* render_group, std::uint32_t index);
    private:
        void destroy() noexcept;

        friend class Shader;
        const RenderGroupsContainer& shader_get_render_groups() const noexcept;
    private:
        Shader* parent_shader;
        const Material* material;
        RenderGroupsContainer render_groups;
        RenderGroupsSearchContainer render_groups_search;
    };
};
