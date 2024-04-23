#pragma once

#include <cstddef>
#include <concepts>
#include <cstdint>
#include <string>
#include <optional>
#include <variant>
#include <vector>

namespace GeometryParser
{
	enum class Element
	{
		VertexXYZ,
		VertexXYZW,
		TextureCoordinatesU,
		TextureCoordinatesUV,
		TextureCoordinatesUVW,
		Normal,
		FaceV,
		FaceVT,
		FaceVN,
		FaceVTN,
		Group
	};

	struct VectorElement
	{
		float data[4];

		bool operator==(const VectorElement &) const noexcept = default;
	};

	struct FaceElement
	{
		std::reference_wrapper<const std::vector<std::uint32_t>> data;

		bool operator==(const FaceElement &fe) const noexcept;
	};

	struct GroupElement
	{
		std::string data;

		GroupElement(std::string_view _data);

		bool operator==(const GroupElement &) const noexcept = default;
	};

	struct ElementData
	{
		Element tag;
		std::variant<VectorElement, FaceElement, GroupElement> data;

		VectorElement & GetVectorElement() noexcept;
		const VectorElement & GetVectorElement() const noexcept;

		FaceElement & GetFaceElement() noexcept;
		const FaceElement & GetFaceElement() const noexcept;

		GroupElement & GetGroupElement() noexcept;
		const GroupElement & GetGroupElement() const noexcept;

		bool operator==(const ElementData &) const noexcept = default;
	};

	enum class FaceType
	{
		Vertex,
		VertexTextureCoordinates,
		VertexNormal,
		VertexTextureCoordinatesNormal
	};

	std::uint8_t FaceTypeToCount(FaceType type) noexcept;

	struct ObjParserSchema
	{
		std::uint8_t vertex_components;
		std::uint8_t texture_coordinates_components;
		FaceType face_type;
		std::uint8_t face_count;

		Element GetVertexElement() const noexcept;
		Element GetTextureCoordinatesElement() const noexcept;
		Element FaceTypeElement() const noexcept;
		std::size_t IndicesPerFace() const noexcept;
	};

	enum class Result
	{
		Success,
		EndOfFile,
		BadFile,
		BadVertex,
		BadTextureCoordinates,
		BadNormal,
		BadFace,
		BadGroup
	};

	struct Error
	{
		Result result;
		std::string str;
		std::size_t column;
	};
};
