#include "PositionTextureNormalTangentObject.h"

namespace FireLand
{
	namespace StaticObject
	{
		std::size_t PositionTextureNormalTangentObject::GetMeshCount() const noexcept
		{
			return meshes.size();
		}

		std::span<const std::byte> PositionTextureNormalTangentObject::GetMeshVertexData() const noexcept
		{
			return {reinterpret_cast<const std::byte *>(vertex_data.data()),
					vertex_data.size() * sizeof(PositionTextureNormalTangent)};
		}

		std::span<const std::uint32_t> PositionTextureNormalTangentObject::GetMeshIndexData() const noexcept
		{
			return {index_data.data(), index_data.size()};
		}

		std::size_t PositionTextureNormalTangentObject::GetMeshIndexDataOffset(std::size_t index) const noexcept
		{
			return meshes[index].index_data_offset;
		}

		const std::vector<PositionTextureNormalTangent> &
		PositionTextureNormalTangentObject::GetVertexData() const noexcept
		{
			return vertex_data;
		}

		const std::vector<std::uint32_t> & PositionTextureNormalTangentObject::GetIndexData() const noexcept
		{
			return index_data;
		}

		const std::vector<Mesh> & PositionTextureNormalTangentObject::GetMeshes() const noexcept
		{
			return meshes;
		}
	};
};
