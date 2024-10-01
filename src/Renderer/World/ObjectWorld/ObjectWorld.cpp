#include "ObjectWorld.h"
#include "../../Allocator/AllocateFromMany.hpp"
#include "../../Context/Device.h"

namespace FireLand
{
    ObjectWorld::ObjectWorld(Device* _parent_device,
                             const std::function<NewPoolSizeCalculator>& _calc)
        : parent_device(_parent_device),
          calc(_calc)
    {}

    ObjectWorld::~ObjectWorld()
    {
        destroy();
    }

    ObjectWorld::ObjectWorld(ObjectWorld&& ow) noexcept
        : parent_device(ow.parent_device),
          calc(ow.calc),
          objects(std::move(ow.objects))
    {}

    ObjectWorld& ObjectWorld::operator=(ObjectWorld&& ow) noexcept
    {
        destroy();

        parent_device = ow.parent_device;
        calc = ow.calc;
        objects = std::move(ow.objects);

        return *this;
    }

    hrs::expected<ObjectPayload, hrs::error>
    ObjectWorld::AcquireObjectPayload(std::size_t vertex_data_size,
                                      std::size_t index_count,
                                      vk::DeviceSize vertex_data_alignment)
    {
        auto vertex_buffer = allocate_buffer(vk::BufferUsageFlagBits::eVertexBuffer,
                                             vertex_data_size,
                                             vertex_data_alignment);

        if(!vertex_buffer)
            return vertex_buffer.error();

        auto index_buffer = allocate_buffer(vk::BufferUsageFlagBits::eVertexBuffer,
                                            index_count * sizeof(std::uint32_t),
                                            sizeof(std::uint32_t));

        if(!index_buffer)
        {
            parent_device->GetAllocator()->Release(vertex_buffer->bounded_buffer,
                                                   MemoryPoolOnEmptyPolicy::Free);

            return index_buffer.error();
        }

        return ObjectPayload(parent_device->GetAllocator(),
                             std::move(vertex_buffer.value()),
                             std::move(index_buffer.value()));
    }

    bool ObjectWorld::AddObject(std::string_view name, std::unique_ptr<Object>&& object)
    {
        if(HasObject(name))
            return false;

        objects.insert(std::pair{name, std::move(object)});
        return true;
    }

    bool ObjectWorld::HasObject(std::string_view name) const noexcept
    {
        return objects.find(name) != objects.end();
    }

    void ObjectWorld::RemoveObject(std::string_view name)
    {
        auto it = objects.find(name);
        if(it != objects.end())
            objects.erase(it);
    }

    Device* ObjectWorld::GetParentDevice() noexcept
    {
        return parent_device;
    }

    const Device* ObjectWorld::GetParentDevice() const noexcept
    {
        return parent_device;
    }

    const std::function<NewPoolSizeCalculator>&
    ObjectWorld::GetNewPoolSizeCalculator() const noexcept
    {
        return calc;
    }

    ObjectWorld::ObjectContainer& ObjectWorld::GetObjects() noexcept
    {
        return objects;
    }

    const ObjectWorld::ObjectContainer& ObjectWorld::GetObjects() const noexcept
    {
        return objects;
    }

    void ObjectWorld::destroy()
    {
        if(!parent_device)
            return;

        objects.clear();
    }

    hrs::expected<BoundedBufferSize, hrs::error>
    ObjectWorld::allocate_buffer(vk::BufferUsageFlags usage,
                                 vk::DeviceSize size,
                                 vk::DeviceSize alignment)
    {
        if(size == 0)
            return BoundedBufferSize{};

        constexpr static std::array variants = {
            MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eDeviceLocal,
                                  MemoryTypeSatisfyOp::Only,
                                  {}),

            MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eDeviceLocal,
                                  MemoryTypeSatisfyOp::Any,
                                  {}),

            MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eDeviceLocal,
                                  MemoryTypeSatisfyOp::Only,
                                  AllocationFlags::AllowPlaceWithMixedResources),

            MemoryPropertyOpFlags(vk::MemoryPropertyFlagBits::eDeviceLocal,
                                  MemoryTypeSatisfyOp::Any,
                                  AllocationFlags::AllowPlaceWithMixedResources),

            MemoryPropertyOpFlags({},
                                  MemoryTypeSatisfyOp::Any,
                                  AllocationFlags::AllowPlaceWithMixedResources)};

        const vk::BufferCreateInfo info({},
                                        size,
                                        usage | vk::BufferUsageFlagBits::eTransferDst,
                                        vk::SharingMode::eExclusive);

        auto buffer_exp =
            AllocateFromMany(*parent_device->GetAllocator(), variants, info, alignment, calc);

        if(!buffer_exp)
            return buffer_exp.error();

        return BoundedBufferSize(std::move(buffer_exp.value()), size);
    }

};
