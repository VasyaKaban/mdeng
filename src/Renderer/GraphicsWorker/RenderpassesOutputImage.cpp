#include "RenderpassesOutputImage.h"
#include "../../Vulkan/UnexpectedVkResult.hpp"
#include "../../Allocator/UnexpectedAllocationResult.hpp"
#include "../../Vulkan/VulkanUtils.hpp"
#include "../../Vulkan/VulkanFormatUtils.hpp"

namespace FireLand
{

	void RenderpassesOutputImage::init(BoundedImage &&_bounded_image, vk::Format _format) noexcept
	{
		bounded_image = std::move(_bounded_image);
		format = _format;
	}

	RenderpassesOutputImage::RenderpassesOutputImage(Allocator *_allocator) noexcept
		: allocator(_allocator)
	{
		hrs::assert_true_debug(_allocator, "Allocator pointer points to null!");
		hrs::assert_true_debug(_allocator->IsCreated(), "Allocator isn't create yet!");
	}

	hrs::unexpected_result
	RenderpassesOutputImage::Recreate(vk::PhysicalDevice ph_device,
									  vk::Format swapchain_format,
									  vk::FormatFeatureFlags format_features,
									  vk::ImageUsageFlags image_usage,
									  const vk::Extent2D &resolution)
	{
		hrs::assert_true_debug(ph_device, "Physical device isn't created yet!");

		Destroy();

		if(IsBadExtent(resolution))
			return {};

		auto best_format_exp = find_best_format(ph_device,
												swapchain_format,
												format_features,
												image_usage,
												resolution);
		if(!best_format_exp)
			return {UnexpectedVkResult(best_format_exp.error())};

		vk::ImageCreateInfo info(CREATE_IMAGE_FLAGS,
								 IMAGE_TYPE,
								 best_format_exp.value(),
								 vk::Extent3D(resolution, 1),
								 MIP_LEVELS,
								 ARRAY_LAYERS,
								 SAMPLES,
								 IMAGE_TILING,
								 image_usage,
								 vk::SharingMode::eExclusive,
								 {},
								 vk::ImageLayout::eUndefined);

		constexpr static std::pair<vk::MemoryPropertyFlagBits, MemoryTypeSatisfyOperation> pairs[] =
		{
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Only},
			{vk::MemoryPropertyFlagBits::eDeviceLocal, MemoryTypeSatisfyOperation::Any},
			{vk::MemoryPropertyFlagBits::eHostVisible, MemoryTypeSatisfyOperation::Any}
		};

		hrs::expected<BoundedImage, AllocationError> buffer_exp;
		for(const auto &pair : pairs)
		{
			buffer_exp = allocator->Create(info,
										   pair.first,
										   {},
										   pair.second);
			if(buffer_exp)
			{
				init(std::move(buffer_exp.value()), best_format_exp.value());
				return {};
			}
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

	hrs::expected<vk::Format, vk::Result>
	RenderpassesOutputImage::find_best_format(vk::PhysicalDevice ph_device,
											  vk::Format swapchain_format,
											  vk::FormatFeatureFlags format_features,
											  vk::ImageUsageFlags image_usage,
											  const vk::Extent2D &resolution) noexcept
	{
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

		auto check_format_support = [ph_device, &resolution, format_features, image_usage]
			(vk::Format format) -> vk::Result
		{
			return IsFormatSatisfyRequirements(ph_device,
											   format,
											   format_features,
											   IMAGE_TYPE,
											   IMAGE_TILING,
											   image_usage,
											   vk::Extent3D(resolution, 1),
											   MIP_LEVELS,
											   ARRAY_LAYERS,
											   SAMPLES,
											   CREATE_IMAGE_FLAGS);
		};

		if(vk::Result res = check_format_support(swapchain_format); res == vk::Result::eSuccess)
			return swapchain_format;
		else if(res == vk::Result::eErrorFormatNotSupported)
		{
			for(const vk::Format format : formats)
			{
				auto res = check_format_support(format);
				if(res == vk::Result::eSuccess)
					return format;
				else if(res != vk::Result::eErrorFormatNotSupported)
					return res;
			}
		}
		else
			return res;

		return vk::Result::eErrorFormatNotSupported;
	}
};
