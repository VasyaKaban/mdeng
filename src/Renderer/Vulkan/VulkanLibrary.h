#pragma once

#include <array>
#include <optional>
#include <span>
#include "hrs/dynamic_library.hpp"
#include "VulkanInclude.h"

namespace FireLand
{
	class VulkanLibrary : public hrs::non_copyable
	{
	public:
#if defined(unix) || defined(__unix) || defined(__unix__)
		constexpr static std::array DEFAULT_NAMES = {"libvulkan.so", "libvulkan.so.1"};
#elif defined(_WIN32) || defined(_WIN64)
		constexpr static std::array DEFAULT_NAMES = {"vulkan-1.dll"};
#endif

		VulkanLibrary() noexcept;
		~VulkanLibrary();
		VulkanLibrary(VulkanLibrary &&vl) noexcept;
		VulkanLibrary & operator=(VulkanLibrary &&vl) noexcept;

		void Close() noexcept;
		std::optional<std::size_t> Open(std::span<const char * const> names) noexcept;

		bool IsOpen() const noexcept;
		explicit operator bool() const noexcept;

		PFN_vkGetInstanceProcAddr GetResolver() const noexcept;

		PFN_vkVoidFunction GetProcAddressRaw(const char *name) const noexcept;

		template<typename P>
			requires std::is_pointer_v<P> && std::is_function_v<std::remove_pointer_t<P>>
		P GetProcAddress(const char *name) const noexcept
		{
			return reinterpret_cast<P>(GetProcAddressRaw(name));
		}

	private:
		hrs::dynamic_library lib;
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
	};
};

