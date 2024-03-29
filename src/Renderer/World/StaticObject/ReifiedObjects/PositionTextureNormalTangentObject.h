#pragma once

#include <vector>
#include "../ReifiedObject.h"
#include "../../../../hrs/math/vector.hpp"
#include "../../../../hrs/math/vector_view.hpp"

namespace FireLand
{
	namespace StaticObject
	{
		struct Mesh
		{
			std::size_t index_data_offset;

			constexpr Mesh(std::size_t _index_data_offset = {}) noexcept
				: index_data_offset(_index_data_offset) {}
		};

		struct PositionTextureNormalTangent
		{
			hrs::math::glsl::vec3 position;
			hrs::math::glsl::vec2 texture;
			hrs::math::glsl::vec3 normal;
			hrs::math::glsl::vec3 tangent;

			constexpr PositionTextureNormalTangent(hrs::math::vector_view<const float, 3> _position = {},
												   hrs::math::vector_view<const float, 2> _texture = {},
												   hrs::math::vector_view<const float, 3> _normal = {},
												   hrs::math::vector_view<const float, 3> _tangent = {}) noexcept
				: position(_position),
				  texture(_texture),
				  normal(_normal),
				  tangent(_tangent) {}
		};

		class PositionTextureNormalTangentObject : public ReifiedObject
		{
		public:
			PositionTextureNormalTangentObject() = default;
			~PositionTextureNormalTangentObject() = default;
			PositionTextureNormalTangentObject(const PositionTextureNormalTangentObject &) = default;
			PositionTextureNormalTangentObject(PositionTextureNormalTangentObject &&) = default;
			PositionTextureNormalTangentObject & operator=(const PositionTextureNormalTangentObject &) = default;
			PositionTextureNormalTangentObject & operator=(PositionTextureNormalTangentObject &&) = default;

			virtual std::size_t GetMeshCount() const noexcept override;
			virtual std::span<const std::byte> GetMeshVertexData() const noexcept override;
			virtual std::span<const std::uint32_t> GetMeshIndexData() const noexcept override;
			virtual std::size_t GetMeshIndexDataOffset(std::size_t index) const noexcept override;

			const std::vector<PositionTextureNormalTangent> & GetVertexData() const noexcept;
			const std::vector<std::uint32_t> & GetIndexData() const noexcept;
			const std::vector<Mesh> & GetMeshes() const noexcept;

		private:
			std::vector<PositionTextureNormalTangent> vertex_data;
			std::vector<std::uint32_t> index_data;
			std::vector<Mesh> meshes;
		};
	};
};
