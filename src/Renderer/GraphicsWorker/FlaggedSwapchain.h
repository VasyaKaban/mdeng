#pragma once

#include "Swapchain/Swapchain.h"
#include "../../hrs/unexpected_result.hpp"

namespace FireLand
{
	enum class SwapchainCreationResult
	{
		UnsupportedImageUsage
	};

	constexpr auto SwapchainCreationResultToString(SwapchainCreationResult result)
	{
		switch(result)
		{
			case SwapchainCreationResult::UnsupportedImageUsage:
				return "UnsupportedImageUsage";
				break;
		}

		return "";
	}

	class UnexpectedSwapchainCreationResult : public hrs::unexpected_result_interface
	{
	public:
		UnexpectedSwapchainCreationResult(SwapchainCreationResult _result,
										  const std::source_location &_location =
										  std::source_location::current()) noexcept
			: result(_result), location(_location) {}

		UnexpectedSwapchainCreationResult(const UnexpectedSwapchainCreationResult &)  = default;
		UnexpectedSwapchainCreationResult(UnexpectedSwapchainCreationResult &&) = default;
		UnexpectedSwapchainCreationResult & operator=(const UnexpectedSwapchainCreationResult &) = default;
		UnexpectedSwapchainCreationResult & operator=(UnexpectedSwapchainCreationResult &&) = default;

		virtual ~UnexpectedSwapchainCreationResult() override = default;

		virtual const std::source_location & GetSourceLocation() const noexcept override
		{
			return location;
		}

		virtual std::string GetErrorMessage() const override
		{
			return SwapchainCreationResultToString(result);
		}

		constexpr SwapchainCreationResult GetResult() const noexcept
		{
			return result;
		}

	private:
		SwapchainCreationResult result;
		std::source_location location;
	};

	class FlaggedSwapchain : public hrs::non_copyable
	{
		constexpr static vk::ImageUsageFlags SWAPCHAIN_USAGE =
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst;

		void init(const Surface *_surface, Swapchain &&_swapchain, bool _is_recreation_needed) noexcept;

	public:
		constexpr static std::uint32_t MIN_SWAPCHAIN_IMAGES_COUNT = 2;

		FlaggedSwapchain(const Device *_device) noexcept;

		hrs::unexpected_result Recreate(const Surface *_surface,
										const vk::Extent2D &resolution,
										vk::PresentModeKHR swapchain_present_mode);

		~FlaggedSwapchain();
		FlaggedSwapchain(FlaggedSwapchain &&) = default;
		FlaggedSwapchain & operator=(FlaggedSwapchain &&) = default;

		void Destroy();
		bool IsCreated() const noexcept;

		Swapchain & GetSwapchain() noexcept;
		const Swapchain & GetSwapchain() const noexcept;
		const Surface * GetSurface() const noexcept;
		bool IsRecreationNeeded() const noexcept;

		void SetRecreationNeed() noexcept;

	private:

		hrs::expected<Swapchain, hrs::unexpected_result>
		create_swapchain(const Device *_device,
						 const Surface *_surface,
						 const vk::Extent2D &resolution,
						 vk::PresentModeKHR swapchain_present_mode,
						 vk::Semaphore _acquire_signal_semaphore = {},
						 vk::Semaphore _present_wait_semaphore = {},
						 vk::SwapchainKHR old_swapchain = {});

		const Device *device;
		const Surface *surface;
		Swapchain swapchain;
		bool is_recreation_needed;
	};
};
