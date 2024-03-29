#pragma once

#include "../../hrs/non_creatable.hpp"
#include "../Context/Device.h"
#include "StaticObject/ReifiedObject.h"
#include "StaticObject/Object.h"
#include "RenderWorld/RenderWorld.h"
#include <map>

namespace FireLand
{
	class World : public hrs::non_copyable
	{
	public:

		World(Device *_parent_device, RenderWorld *_render_world) noexcept;
		~World();
		World(World &&w) noexcept;
		World & operator=(World &&w) noexcept;

		void Destroy();

		Device * GetParentDevice() noexcept;
		const Device * GetParentDevice() const noexcept;
		RenderWorld * GetRenderWorld() noexcept;
		const RenderWorld * GetRenderWorld() const noexcept;

		std::map<const StaticObject::ReifiedObject *, StaticObject::Object> & GetStaticObjects() noexcept;
		void AddStaticObject(const StaticObject::ReifiedObject *robject, const RenderGroupOptions &options);

	private:
		Device *parent_device;
		RenderWorld *render_world;
		std::map<const StaticObject::ReifiedObject *, StaticObject::Object> static_objects;
	};
};
