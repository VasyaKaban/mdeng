#pragma once

#include "../../TransferChannel/Data.h"
#include "../../Vulkan/VulkanInclude.hpp"
#include "ObjectMeshBinding.h"
#include "hrs/block.hpp"
#include "hrs/non_creatable.hpp"
#include <cstdint>
#include <span>
#include <unordered_map>

namespace FireLand
{
    class Object;
    class Shader;
    class ShaderNode;
    class ObjectMeshBinding;
    class RenderWorld;

    struct ShaderDataBinding
    {
        RenderWorld* render_world;
        std::uint32_t index;
    };

    class ObjectInstance : public hrs::non_copyable
    {
    public:
        ObjectInstance(Object* _parent_object);
        virtual ~ObjectInstance();
        ObjectInstance(ObjectInstance&& oi) noexcept;
        ObjectInstance& operator=(ObjectInstance&& oi) noexcept;

        Object* GetParentObejct() noexcept;
        const Object* GetParentObject() const noexcept;

        bool HasShaderDataBinding(const Shader* shader) const noexcept;
        bool AddShaderDataBinding(const Shader* shader, RenderWorld* render_world);
        void UpdateShaderDataBinding(const Shader* shader,
                                     const hrs::block<vk::DeviceSize>& data_block,
                                     vk::DeviceSize in_data_buffer_offset);

        void UpdateAllShaderDataBindings(const hrs::block<vk::DeviceSize>& data_block,
                                         vk::DeviceSize in_data_buffer_offset);

        std::optional<std::uint32_t> GetShaderDataIndex(const Shader* shader) const noexcept;

        std::vector<ObjectMeshBinding>& GetObjectMeshBindings() noexcept;
        const std::vector<ObjectMeshBinding>& GetObjectMeshBindings() const noexcept;
    protected:
        virtual Data GetData() const noexcept = 0;
    private:
        void destroy();
    private:
        Object* parent_object;

        std::unordered_map<const Shader*, ShaderDataBinding> shader_data_bindings;
        std::vector<ObjectMeshBinding> mesh_bindings;
    };
};
