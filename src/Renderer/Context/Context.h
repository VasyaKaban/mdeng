#pragma once

#include <memory>
#include <vector>
#include "hrs/non_creatable.hpp"
#include "GlobalLoader.h"
#include "ContextDestructors.h"
#include "../Vulkan/VulkanLibrary.h"

namespace FireLand
{
	class Instance;

	class Context
	{
	public:
		Context() = default;
		~Context();
		Context(Context &&ctx) noexcept;
		Context & operator=(Context &&ctx) noexcept;

		bool Init(std::span<const char * const> library_names = VulkanLibrary::DEFAULT_NAMES) noexcept;

		void Destroy() noexcept;

		bool IsCreated() const noexcept;
		explicit operator bool() const noexcept;

		const std::string & GetLibraryName() const noexcept;
		const VulkanLibrary & GetLibrary() const noexcept;
		const GlobalLoader & GetGlobalLoader() const noexcept;
		const ContextDestructors & GetContextDestructors() const noexcept;

		void AddInstance(Instance *instance);
		void DeleteInstance(Instance *instance) noexcept;
		bool HasInstance(Instance *instance) const noexcept;

	private:
		VulkanLibrary library;
		std::string library_name;
		GlobalLoader global_loader;
		ContextDestructors context_destructors;
		std::vector<std::unique_ptr<Instance>> instances;
	};
};
