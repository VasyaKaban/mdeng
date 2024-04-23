#pragma once

#include <vector>
#include <cstdint>
#include <filesystem>
#include <map>
#include <functional>
#include <cassert>
#include "hrs/expected.hpp"
#include "hrs/flags.hpp"
#include "ObjSchema.h"

namespace GeometryParser
{
	class ObjParserRange;

	enum class ObjDataLinkFlags
	{
		IgnoreVertexWComponent = 1 << 0,
		VertexWReplace = 1 << 1,
		IgnoreTextureCoordinatesVComponent = 1 << 2,
		IgnoreTextureCoordinatesWComponent = 1 << 3,
		TextureCoordinatesVReplace = 1 << 4,
		TextureCoordinatesWReplace = 1 << 5,
	};

	struct ObjDataReplaceValue
	{
		float vertex_w_replace;
		float texture_coordinates_v_component;
		float texture_coordinates_w_component;
	};

	struct ObjDataFaceComparator
	{
		using ComparatorType = bool (ObjDataFaceComparator::*)(std::size_t, std::size_t) const noexcept;

		const std::vector<std::uint32_t> *face_indices;
		ComparatorType comparator;

		ObjDataFaceComparator() = default;

		constexpr ObjDataFaceComparator(const std::vector<std::uint32_t> &_face_indices,
										FaceType type) noexcept
			: face_indices(&_face_indices),
			  comparator(SelectComparator(type)) {}

		constexpr bool operator()(std::size_t ind1, std::size_t ind2) const noexcept
		{
			return (this->*comparator)(ind1, ind2);
		}

		constexpr bool ComparatorV(std::size_t ind1, std::size_t ind2) const noexcept
		{
			return (*face_indices)[ind1] < (*face_indices)[ind2];
		}

		constexpr bool ComparatorVT(std::size_t ind1, std::size_t ind2) const noexcept
		{
			bool cmp = (*face_indices)[ind1] == (*face_indices)[ind2];
			if(cmp)
				return (*face_indices)[ind1 + 1] < (*face_indices)[ind2 + 1];

			return (*face_indices)[ind1] < (*face_indices)[ind2];
		}

		constexpr bool ComparatorVN(std::size_t ind1, std::size_t ind2) const noexcept
		{
			bool cmp = (*face_indices)[ind1] == (*face_indices)[ind2];
			if(cmp)
				return (*face_indices)[ind1 + 1] < (*face_indices)[ind2 + 1];

			return (*face_indices)[ind1] < (*face_indices)[ind2];
		}

		constexpr bool ComparatorVTN(std::size_t ind1, std::size_t ind2) const noexcept
		{
			bool cmp_v = (*face_indices)[ind1] == (*face_indices)[ind2];
			if(cmp_v)
			{
				bool cmp_t = (*face_indices)[ind1 + 1] == (*face_indices)[ind2 + 1];
				if(cmp_t)
					return (*face_indices)[ind1 + 2] < (*face_indices)[ind2 + 2];

				return (*face_indices)[ind1 + 1] < (*face_indices)[ind2 + 1];
			}

			return (*face_indices)[ind1] < (*face_indices)[ind2];
		}

		constexpr static ComparatorType SelectComparator(FaceType type) noexcept
		{
			switch(type)
			{
				case FaceType::Vertex:
					return &ObjDataFaceComparator::ComparatorV;
					break;
				case FaceType::VertexTextureCoordinates:
					return &ObjDataFaceComparator::ComparatorVT;
					break;
				case FaceType::VertexNormal:
					return &ObjDataFaceComparator::ComparatorVN;
					break;
				case FaceType::VertexTextureCoordinatesNormal:
					return &ObjDataFaceComparator::ComparatorVTN;
					break;
				default:
					assert(false);
					break;
			}
		}
	};

	struct ObjDataGroup
	{
		std::string name;
		std::size_t start_index;
	};

	struct ObjDataSchema
	{
		std::uint8_t vertex_components;
		std::uint8_t texture_components;
		FaceType face_type;
		std::uint8_t face_count;
	};

	class ObjData
	{
	public:
		using UniqueIndexMap = std::map<std::size_t,
										std::uint32_t,
										ObjDataFaceComparator>;


		ObjData() = default;
		~ObjData() = default;
		ObjData(const ObjData &) = default;
		ObjData(ObjData &&) = default;
		ObjData & operator=(const ObjData &) = default;
		ObjData & operator=(ObjData &&) = default;

		ObjData(std::size_t vertices_reserve,
				std::size_t texture_coordinates_reserve,
				std::size_t normals_reserve,
				std::size_t face_indices_reserve,
				std::size_t indices_reserve,
				std::size_t groups_reserve);

		Error Link(const std::filesystem::path &path,
				   bool read_all,
				   hrs::flags<ObjDataLinkFlags> flags,
				   const ObjDataReplaceValue &replace);

		Error Link(const std::string &data,
				   hrs::flags<ObjDataLinkFlags> flags,
				   const ObjDataReplaceValue &replace);

		Error Link(std::string &&data,
				   hrs::flags<ObjDataLinkFlags> flags,
				   const ObjDataReplaceValue &replace);

		Error Link(ObjParserRange &rng,
				   hrs::flags<ObjDataLinkFlags> flags,
				   const ObjDataReplaceValue &replace,
				   bool close_range);

		void Clear() noexcept;
		void ClearUniqueFaces() noexcept;
		void ConstructUniqueFaces() noexcept;

		const std::vector<float> & GetVertices() const noexcept;
		const std::vector<float> & GetTextureCoordinates() const noexcept;
		const std::vector<float> & GetNormals() const noexcept;
		const std::vector<std::uint32_t> & GetFaceIndices() const noexcept;
		const std::vector<std::uint32_t> & GetIndices() const noexcept;
		const std::vector<ObjDataGroup> & GetGroups() const noexcept;
		const ObjDataSchema & GetSchema() const noexcept;
		const UniqueIndexMap & GetUniqueFaceIndices() const noexcept;

		const float * GetVertexByIndex(std::size_t index) const noexcept;
		const float * GetTextureCoordinatesByIndex(std::size_t index) const noexcept;
		const float * GetNormalByIndex(std::size_t index) const noexcept;

		std::pair<const std::uint32_t *, std::uint32_t>
		GetFaceIndexIdPair(std::size_t index) const noexcept;

	private:
		std::vector<float> vertices;
		std::vector<float> texture_coordinates;
		std::vector<float> normals;
		std::vector<std::uint32_t> face_indices;//like a 1/2/3 or 2//3
		std::vector<std::uint32_t> indices;
		std::vector<ObjDataGroup> groups;
		ObjDataSchema schema;
		UniqueIndexMap unique_faces;
	};
};
