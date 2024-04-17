#pragma once

#include <cstdint>
#include <unordered_map>
#include "../../../hrs/non_creatable.hpp"

namespace FireLand
{
	class ObjectInstance;
	class Mesh;
	class Material;
	class RenderGroup;
	class RenderWorld;

	struct RenderBinding
	{
		const RenderGroup *render_group;
		RenderWorld *render_world;
		std::uint32_t index;
	};

	class ObjectMeshBinding : public hrs::non_copyable
	{
	public:
		ObjectMeshBinding(ObjectInstance *_parent_object_instance, const Mesh *_mesh) noexcept;
		~ObjectMeshBinding();
		ObjectMeshBinding(ObjectMeshBinding &&omb) noexcept;
		ObjectMeshBinding & operator=(ObjectMeshBinding &&omb) noexcept;

		ObjectInstance * GetParentObjectInstance() noexcept;
		const ObjectInstance * GetParentObjectInstance() const noexcept;
		const Mesh * GetMesh() const noexcept;

		bool HasRenderBinding(const RenderGroup *render_group) const noexcept;
		void RemoveRenderBinding(const RenderGroup *render_group) noexcept;
		bool AddRenderBinding(const RenderGroup *render_group, RenderWorld *render_world);

	private:
		void destroy();
	private:
		ObjectInstance *parent_object_instance;

		const Mesh *mesh;
		std::unordered_map<const RenderGroup *, RenderBinding> render_bindings;
	};
};
