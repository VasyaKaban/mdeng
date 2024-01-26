#include "RenderpassesOutputImage.h"
#include "../../Vulkan/UnexpectedVkResult.hpp"
#include "../../Allocator/UnexpectedAllocationResult.hpp"
#include "../../Vulkan/VulkanUtils.hpp"

namespace FireLand
{
	RenderpassesOutputImage::RenderpassesOutputImage(Allocator *_allocator,
													BoundedImage &&_bounded_image,
													vk::Format _format) noexcept
		: allocator(_allocator),
		  bounded_image(std::move(_bounded_image)),
		  format(_format) {}

	hrs::expected<RenderpassesOutputImage, hrs::unexpected_result>
	RenderpassesOutputImage::Create(Allocator *_allocator,
									vk::PhysicalDevice ph_device,
									vk::Format swapchain_format,
									const vk::Extent2D &resolution)
	{
		hrs::assert_true_debug(_allocator, "Allocator pointer points to null!");
		hrs::assert_true_debug(_allocator->IsCreated(), "Allocator isn't create yet!");

		if(IsBadExtent(resolution))
			return RenderpassesOutputImage(_allocator, {}, vk::Format::eUndefined);

		constexpr static std::array formats =
		{
			vk::Format::eR8G8B8A8Unorm,
			vk::Format::eR8G8B8A8Snorm,
			vk::Format::eR8G8B8A8Uscaled,
			vk::Format::eR8G8B8A8Sscaled,
			vk::Format::eR8G8B8A8Uint,
			vk::Format::eR8G8B8A8Sint,
			vk::Format::eR8G8B8A8Srgb,
			vk::Format::eB8G8R8A8Unorm,
			vk::Format::eB8G8R8A8Snorm,
			vk::Format::eB8G8R8A8Uscaled,
			vk::Format::eB8G8R8A8Sscaled,
			vk::Format::eB8G8R8A8Uint,
			vk::Format::eB8G8R8A8Sint,
			vk::Format::eB8G8R8A8Srgb,
			vk::Format::eA8B8G8R8UnormPack32,
			vk::Format::eA8B8G8R8SnormPack32,
			vk::Format::eA8B8G8R8UscaledPack32,
			vk::Format::eA8B8G8R8SscaledPack32,
			vk::Format::eA8B8G8R8UintPack32,
			vk::Format::eA8B8G8R8SintPack32,
			vk::Format::eA8B8G8R8SrgbPack32,
			vk::Format::eA2R10G10B10UnormPack32,
			vk::Format::eA2R10G10B10SnormPack32,
			vk::Format::eA2R10G10B10UscaledPack32,
			vk::Format::eA2R10G10B10SscaledPack32,
			vk::Format::eA2R10G10B10UintPack32,
			vk::Format::eA2R10G10B10SintPack32,
			vk::Format::eA2B10G10R10UnormPack32,
			vk::Format::eA2B10G10R10SnormPack32,
			vk::Format::eA2B10G10R10UscaledPack32,
			vk::Format::eA2B10G10R10SscaledPack32,
			vk::Format::eA2B10G10R10UintPack32,
			vk::Format::eA2B10G10R10SintPack32
		};

		constexpr static vk::FormatFeatureFlags desired_format_features =
			vk::FormatFeatureFlagBits::eColorAttachmentBlend;

		vk::Format choosed_format = vk::Format::eUndefined;

		auto swapchain_format_features = ph_device.getFormatProperties(swapchain_format);
		if(swapchain_format_features.optimalTilingFeatures & desired_format_features)
			choosed_format = swapchain_format;

		if(choosed_format == vk::Format::eUndefined)
		{
			for(const vk::Format format : formats)
			{
				auto format_features = ph_device.getFormatProperties(format);
				if(format_features.optimalTilingFeatures & desired_format_features)
				{
					choosed_format = format;
					break;
				}
			}
		}

		if(choosed_format == vk::Format::eUndefined)
			return {UnexpectedVkResult(vk::Result::eErrorFormatNotSupported)};

		constexpr vk::ImageUsageFlags image_usage =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eInputAttachment;

		vk::ImageCreateInfo info({},
								 vk::ImageType::e2D,
								 choosed_format,
								 vk::Extent3D(resolution, 1),
								 1,
								 1,
								 vk::SampleCountFlagBits::e1,
								 vk::ImageTiling::eOptimal,
								 image_usage,
								 vk::SharingMode::eExclusive,
								 {},
								 vk::ImageLayout::eColorAttachmentOptimal);

		constexpr static std::pair<vk::MemoryPropertyFlagBits, MemoryTypeSatisfyOperation> pairs[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Only},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Any},
			{vk::MemoryPropertyFlagBits::eHostVisible, MemoryTypeSatisfyOperation::Any}
		};

		hrs::expected<BoundedImage, AllocationError> buffer_exp;
		for(const auto &pair : pairs)
		{
			buffer_exp = _allocator->Create(info,
											pair.first,
											{},
											pair.second);
			if(buffer_exp)
				return RenderpassesOutputImage(_allocator, std::move(buffer_exp.value()), choosed_format);
			else if(buffer_exp.error().keeps<vk::Result>())
				return {UnexpectedVkResult(buffer_exp.error().get<vk::Result>())};
		}

		if(buffer_exp.error().keeps<vk::Result>())
			return {UnexpectedVkResult(buffer_exp.error().get<vk::Result>())};
		else
			return {UnexpectedAllocationResult(buffer_exp.error().get<AllocationResult>())};
	}

	RenderpassesOutputImage::~RenderpassesOutputImage()
	{
		Destroy();
	}

	RenderpassesOutputImage::RenderpassesOutputImage(RenderpassesOutputImage &&out_img) noexcept
		: allocator(out_img.allocator),
		  bounded_image(std::move(out_img.bounded_image)),
		  format(out_img.format)
	{
		out_img.bounded_image = {};
	}

	RenderpassesOutputImage & RenderpassesOutputImage::operator=(RenderpassesOutputImage &&out_img) noexcept
	{
		Destroy();

		allocator = out_img.allocator;
		bounded_image = std::move(out_img.bounded_image);
		format = out_img.format;

		out_img.bounded_image = {};

		return *this;
	}

	void RenderpassesOutputImage::Destroy()
	{
		if(!IsCreated())
			return;

		allocator->Destroy(bounded_image, false);
		bounded_image = {};
	}

	bool RenderpassesOutputImage::IsCreated() const noexcept
	{
		return bounded_image.IsCreated();
	}

	const BoundedImage & RenderpassesOutputImage::GetBoundedImage() const noexcept
	{
		return bounded_image;
	}

	vk::Format RenderpassesOutputImage::GetFormat() const noexcept
	{
		return format;
	}
};
