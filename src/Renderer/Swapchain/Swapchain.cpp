#include "Swapchain.h"
#include "../Context/Device.h"
#include "../Context/Surface.h"
#include "hrs/scoped_call.hpp"

namespace FireLand
{
    RetiredSwapchain::RetiredSwapchain(Device* _parent_device, vk::SwapchainKHR _swapchain) noexcept
        : parent_device(_parent_device),
          swapchain(_swapchain)
    {}

    RetiredSwapchain::~RetiredSwapchain()
    {
        Destroy();
    }

    RetiredSwapchain::RetiredSwapchain(RetiredSwapchain&& rsc) noexcept
        : parent_device(rsc.parent_device),
          swapchain(std::exchange(rsc.swapchain, VK_NULL_HANDLE))
    {}

    RetiredSwapchain& RetiredSwapchain::operator=(RetiredSwapchain&& rsc) noexcept
    {
        Destroy();

        parent_device = rsc.parent_device;
        swapchain = std::exchange(rsc.swapchain, VK_NULL_HANDLE);

        return *this;
    }

    void RetiredSwapchain::Destroy() noexcept
    {
        if(*this)
        {
            parent_device->GetHandle().destroy(swapchain);
            swapchain = VK_NULL_HANDLE;
        }
    }

    RetiredSwapchain::operator bool() const noexcept
    {
        return IsCreated();
    }

    bool RetiredSwapchain::IsCreated() const noexcept
    {
        return parent_device && swapchain;
    }

    Device* RetiredSwapchain::GetParentDevice() noexcept
    {
        return parent_device;
    }
    const Device* RetiredSwapchain::GetParentDevice() const noexcept
    {
        return parent_device;
    }

    vk::SwapchainKHR RetiredSwapchain::GetHandle() const noexcept
    {
        return swapchain;
    }

    vk::SwapchainKHR RetiredSwapchain::Release() noexcept
    {
        return std::exchange(swapchain, VK_NULL_HANDLE);
    }

    Swapchain::Swapchain(Device* _parent_device) noexcept
        : parent_device(_parent_device)
    {}

    Swapchain::~Swapchain()
    {
        Destroy();
    }

    Swapchain::Swapchain(Swapchain&& sc) noexcept
        : parent_device(sc.parent_device),
          surface(sc.surface),
          swapchain(std::exchange(sc.swapchain, VK_NULL_HANDLE)),
          images(std::move(sc.images)),
          resolution(sc.resolution),
          present_mode(sc.present_mode),
          surface_format(sc.surface_format),
          present_queue(sc.present_queue),
          acquire_signal_semaphores(std::move(sc.acquire_signal_semaphores)),
          acquire_index(sc.acquire_index)
    {}

    Swapchain& Swapchain::operator=(Swapchain&& sc) noexcept
    {
        Destroy();

        parent_device = sc.parent_device;
        surface = sc.surface;
        swapchain = std::exchange(sc.swapchain, VK_NULL_HANDLE);
        images = std::move(sc.images);
        resolution = sc.resolution;
        present_mode = sc.present_mode;
        surface_format = sc.surface_format;
        present_queue = sc.present_queue;
        acquire_signal_semaphores = std::move(sc.acquire_signal_semaphores);
        acquire_index = sc.acquire_index;

        return *this;
    }

    vk::Result Swapchain::Recreate(const SwapchainRecreateInfo& info,
                                   const RetiredSwapchain& retired) noexcept
    {
        Destroy();

        if(info.resolution.width == 0 || info.resolution.height == 0)
            return vk::Result::eSuccess;

        vk::Device device_handle = parent_device->GetHandle();
        vk::SwapchainKHR old_swapchain = retired.GetHandle();
        const vk::SwapchainCreateInfoKHR swapchain_info(info.flags,
                                                        info.surface->GetHandle(),
                                                        info.min_image_count,
                                                        info.surface_format.format,
                                                        info.surface_format.colorSpace,
                                                        info.resolution,
                                                        info.image_array_layers,
                                                        info.image_usage,
                                                        vk::SharingMode::eExclusive,
                                                        0,
                                                        nullptr,
                                                        info.pre_transform,
                                                        info.composite_alpha,
                                                        info.present_mode,
                                                        info.clipped,
                                                        old_swapchain);

        auto [u_swapchain_res, u_swapchain] =
            device_handle.createSwapchainKHRUnique(swapchain_info);
        if(u_swapchain_res != vk::Result::eSuccess)
            return u_swapchain_res;

        auto [_images_res, _images] = device_handle.getSwapchainImagesKHR(u_swapchain.get());
        if(_images_res != vk::Result::eSuccess)
            return _images_res;

        std::vector<vk::Semaphore> _acquire_signal_semaphores;
        hrs::scoped_call semaphores_dtor = [device_handle, &_acquire_signal_semaphores]
        {
            for(const auto sem: _acquire_signal_semaphores)
                device_handle.destroy(sem);
        };

        _acquire_signal_semaphores.reserve(info.min_image_count);
        const vk::SemaphoreCreateInfo sem_info{};
        for(std::uint32_t i = 0; i < info.min_image_count; i++)
        {
            auto [sem_res, sem] = device_handle.createSemaphore(sem_info);
            if(sem_res != vk::Result::eSuccess)
                return sem_res;

            _acquire_signal_semaphores.push_back(sem);
        }

        surface = info.surface;
        swapchain = u_swapchain.release();
        images = std::move(_images);
        resolution = info.resolution;
        present_mode = info.present_mode;
        surface_format = info.surface_format;
        present_queue = info.present_queue;
        acquire_signal_semaphores = std::move(_acquire_signal_semaphores);
        acquire_index = 0;

        semaphores_dtor.drop();
        return vk::Result::eSuccess;
    }

    void Swapchain::Destroy() noexcept
    {
        if(!parent_device)
            return;

        vk::Device device_handle = parent_device->GetHandle();
        ;

        if(!IsCreated())
        {
            if(!IsRetired())
                return;

            images.clear();
            for(const auto sem: acquire_signal_semaphores)
                device_handle.destroy(sem);

            acquire_signal_semaphores.clear();
        }

        device_handle.destroy(swapchain);
        swapchain = VK_NULL_HANDLE;
    }

    Swapchain::operator bool() const noexcept
    {
        return IsCreated();
    }

    bool Swapchain::IsCreated() const noexcept
    {
        return parent_device && swapchain;
    }

    bool Swapchain::IsRetired() const noexcept
    {
        return !IsCreated() && !images.empty();
    }

    hrs::expected<std::pair<std::uint32_t, vk::Semaphore>, vk::Result>
    Swapchain::AcquireImage(vk::Fence fence, std::uint64_t timeout) noexcept
    {
        std::uint32_t index = acquire_index;
        acquire_index = (acquire_index + 1) % images.size();
        auto [res, image_index] =
            parent_device->GetHandle().acquireNextImageKHR(swapchain,
                                                           timeout,
                                                           acquire_signal_semaphores[index],
                                                           fence);

        switch(res)
        {
            case vk::Result::eSuccess:
            case vk::Result::eSuboptimalKHR:
                return std::pair{image_index, acquire_signal_semaphores[index]};
                break;
            default:
                acquire_index = index;
                return res;
                break;
        }
    }

    vk::Result Swapchain::Present(std::span<const vk::Semaphore> wait_semaphores,
                                  std::span<const std::uint32_t> image_indices) noexcept
    {
        const vk::PresentInfoKHR info(wait_semaphores, swapchain, image_indices);
        return present_queue.presentKHR(info);
    }

    RetiredSwapchain Swapchain::Retire() noexcept
    {
        return {parent_device, std::exchange(swapchain, VK_NULL_HANDLE)};
    }

    Device* Swapchain::GetParentDevice() noexcept
    {
        return parent_device;
    }

    const Device* Swapchain::GetParentDevice() const noexcept
    {
        return parent_device;
    }
    const Surface* Swapchain::GetSurface() const noexcept
    {
        return surface;
    }

    vk::SwapchainKHR Swapchain::GetHandle() const noexcept
    {
        return swapchain;
    }

    const std::vector<vk::Image>& Swapchain::GetImages() const noexcept
    {
        return images;
    }

    const vk::Extent2D& Swapchain::GetResolution() const noexcept
    {
        return resolution;
    }

    vk::PresentModeKHR Swapchain::GetPresentMode() const noexcept
    {
        return present_mode;
    }

    const vk::SurfaceFormatKHR& Swapchain::GetSurfaceFormat() const noexcept
    {
        return surface_format;
    }

    vk::Queue Swapchain::GetPresentQueue() const noexcept
    {
        return present_queue;
    }
};
