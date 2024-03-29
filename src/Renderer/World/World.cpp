#include "World.h"

namespace FireLand
{
	World::World(Device *_parent_device, RenderWorld *_render_world) noexcept
		: parent_device(_parent_device),
		  render_world(_render_world) {}

	World::~World()
	{
		Destroy();
	}

	World::World(World &&w) noexcept
		: parent_device(w.parent_device),
		  static_objects(std::move(w.static_objects)) {}

	World & World::operator=(World &&w) noexcept
	{
		Destroy();

		parent_device = w.parent_device;
		static_objects = std::move(w.static_objects);

		return *this;
	}

	void World::Destroy()
	{
		static_objects.clear();
	}

	Device * World::GetParentDevice() noexcept
	{
		return parent_device;
	}

	const Device * World::GetParentDevice() const noexcept
	{
		return parent_device;
	}

	RenderWorld * World::GetRenderWorld() noexcept
	{
		return render_world;
	}

	const RenderWorld * World::GetRenderWorld() const noexcept
	{
		return render_world;
	}

	std::map<const StaticObject::ReifiedObject *, StaticObject::Object> & World::GetStaticObjects() noexcept
	{
		return static_objects;
	}

	void World::AddStaticObject(const StaticObject::ReifiedObject *robject, const RenderGroupOptions &options)
	{
		auto it = static_objects.find(robject);
		if(it != static_objects.end())
			return;

		static_objects.emplace(robject, robject, this, options);
	}
};
