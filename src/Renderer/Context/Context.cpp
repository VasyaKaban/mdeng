#include "Context.h"

namespace FireLand
{
	Context::~Context()
	{
		Destroy();
	}

	Context::Context(Context &&ctx) noexcept
		: library(std::move(ctx.library)),
		  library_name(std::move(ctx.library_name)),
		  global_loader(ctx.global_loader),
		  context_destructors(ctx.context_destructors),
		  instances(std::move(ctx.instances)) {}

	Context & Context::operator=(Context &&ctx) noexcept
	{
		Destroy();

		library = std::move(ctx.library);
		library_name = std::move(ctx.library_name);
		global_loader = ctx.global_loader;
		context_destructors = ctx.context_destructors;
		instances = std::move(ctx.instances);

		return *this;
	}

	bool Context::Init(std::span<const char * const> library_names) noexcept
	{
		Destroy();

		VulkanLibrary _library;
		auto lib_index_opt = _library.Open(library_names);
		if(!lib_index_opt)
			return false;

		GlobalLoader _global_loader;
		bool loader_init = _global_loader.Init(_library);
		if(!loader_init)
			return false;

		library = std::move(_library);
		library_name = library_names[*lib_index_opt];
		global_loader = _global_loader;
		context_destructors.Init(global_loader.vkGetInstanceProcAddr);

		return true;
	}

	void Context::Destroy() noexcept
	{
		if(!IsCreated())
			return;

		instances.clear();
		library.Close();
	}

	bool Context::IsCreated() const noexcept
	{
		return library.IsOpen();
	}

	Context::operator bool() const noexcept
	{
		return IsCreated();
	}

	const std::string & Context::GetLibraryName() const noexcept
	{
		return library_name;
	}

	const VulkanLibrary & Context::GetLibrary() const noexcept
	{
		return library;
	}

	const GlobalLoader & Context::GetGlobalLoader() const noexcept
	{
		return global_loader;
	}

	const ContextDestructors & Context::GetContextDestructors() const noexcept
	{
		return context_destructors;
	}

	void Context::AddInstance(Instance *instance)
	{
		for(const auto &i : instances)
			if(i.get() == instance)
				return;

		instances.push_back(std::unique_ptr<Instance>(instance));
	}

	void Context::DeleteInstance(Instance *instance) noexcept
	{
		auto it = std::ranges::find_if(instances, [instance](const std::unique_ptr<Instance> &i)
		{
			return i.get() == instance;
		});

		if(it != instances.end())
			instances.erase(it);
	}

	bool Context::HasInstance(Instance *instance) const noexcept
	{
		auto it = std::ranges::find_if(instances, [instance](const std::unique_ptr<Instance> &i)
		{
			return i.get() == instance;
		});

		return it != instances.end();
	}
};
