#include "PostProcessImages.h"
#include "../../../Vulkan/UnexpectedVkResult.hpp"
#include "../../../Allocator/UnexpectedAllocationResult.hpp"
#include "../../../Vulkan/VulkanUtils.hpp"
#include "../../../hrs/scoped_call.hpp"

namespace FireLand
{
	void PostProcessImages::init(std::array<BoundedImage, 2> &&_images,
								 vk::Sampler _sampler,
								 vk::Format _format,
								 const vk::Extent2D &_resolution) noexcept
	{
		images = std::move(_images);
		sampler = _sampler;
		format = _format;
		resolution = _resolution;
	}

	PostProcessImages::PostProcessImages(Device *_parent_device) noexcept
		: parent_device(_parent_device)
	{
		hrs::assert_true_debug(_parent_device, "Parent device pointer points to null!");
		hrs::assert_true_debug(_parent_device->GetHandle(), "Parend device isn't created yet!");
	}

	PostProcessImages::~PostProcessImages()
	{
		Destroy();
	}

	PostProcessImages::PostProcessImages(PostProcessImages &&ppi) noexcept
		: parent_device(ppi.parent_device),
		  images(ppi.images),
		  sampler(std::exchange(ppi.sampler, VK_NULL_HANDLE)),
		  format(ppi.format),
		  resolution(ppi.resolution),
		  target_output_image(std::exchange(ppi.target_output_image, 0))

	{
		ppi.images = {};
	}

	PostProcessImages & PostProcessImages::operator=(PostProcessImages &&ppi) noexcept
	{
		Destroy();

		parent_device = ppi.parent_device;
		images = ppi.images;
		sampler = std::exchange(ppi.sampler, VK_NULL_HANDLE);
		format = ppi.format;
		resolution = ppi.resolution;
		target_output_image = std::exchange(ppi.target_output_image, 0);

		ppi.images = {};

		return *this;
	}

	hrs::unexpected_result
	PostProcessImages::Recreate(const vk::SamplerCreateInfo &sampler_info,
								vk::Format _format,
								vk::ImageUsageFlags usage,
								vk::ImageLayout initial_layout,
								const vk::Extent2D &_resolution)
	{
		Destroy();

		if(IsBadExtent(_resolution))
		{
			resolution = vk::Extent2D{};
			return {};
		}

		auto [u_sampler_res, u_sampler] = parent_device->GetHandle().createSamplerUnique(sampler_info);
		if(u_sampler_res != vk::Result::eSuccess)
			return UnexpectedVkResult(u_sampler_res);

		auto images_exp = allocate_images(_format,
										  usage,
										  initial_layout,
										  _resolution);

		if(!images_exp)
			return images_exp.error();


		init(std::move(images_exp.value()),
			 u_sampler.release(),
			 _format,
			 _resolution);

		return {};
	}

	void PostProcessImages::Destroy()
	{
		if(!IsCreated())
			return;

		parent_device->GetHandle().destroy(sampler);
		for(const auto &img : images)
			parent_device->GetAllocator()->Destroy(img, false);

		images = {};
		sampler = VK_NULL_HANDLE;
		target_output_image = 0;
	}

	bool PostProcessImages::IsCreated() const noexcept
	{
		return sampler;
	}

	Device * PostProcessImages::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const Device * PostProcessImages::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	const std::array<BoundedImage, 2> & PostProcessImages::GetBoundedImages() const noexcept
	{
		return images;
	}

	vk::Sampler PostProcessImages::GetSampler() const noexcept
	{
		return sampler;
	}

	vk::Format PostProcessImages::GetFormat() const noexcept
	{
		return format;
	}

	std::size_t PostProcessImages::GetTargetOutputImageIndex() const noexcept
	{
		return target_output_image;
	}

	vk::Image PostProcessImages::GetOutputImage() const noexcept
	{
		return images[target_output_image].image;
	}

	vk::Image PostProcessImages::GetInputImage() const noexcept
	{
		return images[1 - target_output_image].image;
	}

	void PostProcessImages::SwitchImages() noexcept
	{
		target_output_image = 1 - target_output_image;
	}

	void PostProcessImages::ResetTargetOutputImageIndex() noexcept
	{
		target_output_image = 0;
	}

	hrs::expected<std::array<BoundedImage, 2>, hrs::unexpected_result>
	PostProcessImages::allocate_images(vk::Format _format,
									   vk::ImageUsageFlags usage,
									   vk::ImageLayout initial_layout,
									   const vk::Extent2D &_resolution)
	{
		constexpr static std::pair<vk::MemoryPropertyFlagBits, MemoryTypeSatisfyOperation> pairs[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Only},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Any},
			{vk::MemoryPropertyFlagBits{}, MemoryTypeSatisfyOperation::Any}
		};

		Allocator *allocator = parent_device->GetAllocator();
		std::array<BoundedImage, 2> out_images;
		hrs::scoped_call out_images_dtor([&out_images, allocator]()
		{
			for(const auto &img : out_images)
			{
				if(img.IsCreated())
					allocator->Destroy(img, false);
				else
					break;
			}
		});

		const vk::ImageCreateInfo info({},
									   vk::ImageType::e2D,
									   _format,
									   vk::Extent3D(_resolution, 1),
									   1,
									   1,
									   vk::SampleCountFlagBits::e1,
									   vk::ImageTiling::eOptimal,
									   usage,
									   vk::SharingMode::eExclusive,
									   {},
									   initial_layout);

		hrs::unexpected_result unexp_res;
		for(std::size_t i = 0; i < 2; i++)
		{
			for(const auto &pair : pairs)
			{
				auto image_exp = allocator->Create(info,
												   pair.first,
												   {},
												   pair.second);

				if(!image_exp)
				{
					if(image_exp.error().keeps<vk::Result>())
						return {UnexpectedVkResult(image_exp.error().get<vk::Result>())};
					else
						unexp_res = UnexpectedAllocationResult(image_exp.error().get<AllocationResult>());
				}
				else
				{
					out_images[i] = image_exp.value();
					break;
				}
			}

			if(!out_images[i].IsCreated())
				return unexp_res;
		}

		out_images_dtor.Drop();
		return out_images;
	}
};
