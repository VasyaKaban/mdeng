#include "ObjData.h"
#include "ObjParserRange.h"
#include <map>

namespace GeometryParser
{
	ObjData::ObjData(std::size_t vertices_reserve,
					 std::size_t texture_coordinates_reserve,
					 std::size_t normals_reserve,
					 std::size_t face_indices_reserve,
					 std::size_t indices_reserve,
					 std::size_t groups_reserve)
	{
		vertices.reserve(vertices_reserve);
		texture_coordinates.reserve(texture_coordinates_reserve);
		normals.reserve(normals_reserve);
		face_indices.reserve(face_indices_reserve);
		indices.reserve(indices_reserve);
		groups.reserve(groups_reserve);
	}


	Error ObjData::Link(const std::filesystem::path &path,
						bool read_all,
						hrs::flags<ObjDataLinkFlags> flags,
						const ObjDataReplaceValue &replace)
	{
		ObjParserRange rng;
		auto res = rng.Open(path, read_all);
		if(res != Result::Success)
			return Error{.result = res};

		return Link(rng, flags, replace, true);
	}

	Error ObjData::Link(const std::string &data,
						hrs::flags<ObjDataLinkFlags> flags,
						const ObjDataReplaceValue &replace)
	{
		ObjParserRange rng;
		auto res = rng.Consume(data);
		if(res != Result::Success)
			return Error{.result = res};

		return Link(rng, flags, replace, true);
	}

	Error ObjData::Link(std::string &&data,
						hrs::flags<ObjDataLinkFlags> flags,
						const ObjDataReplaceValue &replace)
	{
		ObjParserRange rng;
		auto res = rng.Consume(std::move(data));
		if(res != Result::Success)
			return Error{.result = res};

		return Link(rng, flags, replace, true);
	}

	Error ObjData::Link(ObjParserRange &rng,
						hrs::flags<ObjDataLinkFlags> flags,
						const ObjDataReplaceValue &replace,
						bool close_range)
	{
		Clear();
		schema.vertex_components = 3;
		schema.texture_components = 1;

		std::uint32_t id = 0;

		auto exp = rng.Next();
		for(; exp.has_value(); exp = rng.Next())
		{
			const auto &elem = exp.value();
			switch(elem.tag)
			{
				case Element::VertexXYZ:
					for(std::size_t i = 0; i < 3; i++)
						vertices.push_back(elem.GetVectorElement().data[i]);

					if(flags & ObjDataLinkFlags::VertexWReplace)
					{
						vertices.push_back(replace.vertex_w_replace);
						schema.vertex_components = 4;
					}
					break;
				case Element::VertexXYZW:
					{
						for(std::size_t i = 0; i < 3; i++)
							vertices.push_back(elem.GetVectorElement().data[i]);

						if(!(flags & ObjDataLinkFlags::IgnoreVertexWComponent))
						{
							vertices.push_back(elem.GetVectorElement().data[3]);
							schema.vertex_components = 4;
						}
						else if(flags & ObjDataLinkFlags::VertexWReplace)
						{
							vertices.push_back(replace.vertex_w_replace);
							schema.vertex_components = 4;
						}
					}
					break;
				case Element::TextureCoordinatesU:
					texture_coordinates.push_back(elem.GetVectorElement().data[0]);

					if(flags & ObjDataLinkFlags::TextureCoordinatesVReplace)
					{
						texture_coordinates.push_back(replace.texture_coordinates_v_component);
						schema.texture_components = 2;

						if(flags & ObjDataLinkFlags::TextureCoordinatesWReplace)
						{
							texture_coordinates.push_back(replace.texture_coordinates_w_component);
							schema.texture_components = 3;
						}
					}
					break;
				case Element::TextureCoordinatesUV:
					{
						bool has_v = false;
						texture_coordinates.push_back(elem.GetVectorElement().data[0]);
						if(!(flags & ObjDataLinkFlags::IgnoreTextureCoordinatesVComponent))
						{
							texture_coordinates.push_back(elem.GetVectorElement().data[1]);
							has_v = true;
							schema.texture_components = 2;
						}
						else if(flags & ObjDataLinkFlags::TextureCoordinatesVReplace)
						{
							texture_coordinates.push_back(replace.texture_coordinates_v_component);
							has_v = true;
							schema.texture_components = 2;
						}

						if(has_v && (flags & ObjDataLinkFlags::TextureCoordinatesWReplace))
						{
							texture_coordinates.push_back(replace.texture_coordinates_w_component);
							schema.texture_components = 3;
						}
					}
					break;
				case Element::TextureCoordinatesUVW:
					{
						bool has_v = false;
						texture_coordinates.push_back(elem.GetVectorElement().data[0]);
						if(!(flags & ObjDataLinkFlags::IgnoreTextureCoordinatesVComponent))
						{
							texture_coordinates.push_back(elem.GetVectorElement().data[1]);
							has_v = true;
							schema.texture_components = 2;
						}
						else if(flags & ObjDataLinkFlags::TextureCoordinatesVReplace)
						{
							texture_coordinates.push_back(replace.texture_coordinates_v_component);
							has_v = true;
							schema.texture_components = 2;
						}

						if(has_v)
						{
							if(!(flags & ObjDataLinkFlags::IgnoreTextureCoordinatesWComponent))
							{
								texture_coordinates.push_back(elem.GetVectorElement().data[2]);
								schema.texture_components = 3;
							}
							else if(flags & ObjDataLinkFlags::TextureCoordinatesWReplace)
							{
								texture_coordinates.push_back(replace.texture_coordinates_w_component);
								schema.texture_components = 3;
							}
						}
					}
					break;
				case Element::Normal:
					for(std::size_t i = 0; i < 3; i++)
						normals.push_back(elem.GetVectorElement().data[i]);
					break;
				case Element::Group:
					groups.push_back({elem.GetGroupElement().data, indices.size()});
					break;
				case Element::FaceV:
				case Element::FaceVT:
				case Element::FaceVN:
				case Element::FaceVTN:
					{
						if(unique_faces.empty())
						{
							schema.face_type = rng.GetSchema().face_type;
							unique_faces = UniqueIndexMap(ObjDataFaceComparator(face_indices,
																				schema.face_type));
						}

						std::size_t i = 0;
						while(i != elem.GetFaceElement().data.get().size())
						{
							std::size_t index = face_indices.size();
							for(int j = 0; j < FaceTypeToCount(schema.face_type); j++)
								face_indices.push_back(elem.GetFaceElement().data.get().data()[i + j]);

							auto it = unique_faces.insert({index, 0});
							if(it.second)//inserted -> add vertex data
								it.first->second = id++;

							indices.push_back(it.first->second);

							i += FaceTypeToCount(schema.face_type);
						}
					}
					break;
			}
		}

		schema.face_count = rng.GetSchema().face_count;

		if(exp.error().result != Result::EndOfFile)
			return exp.error();

		if(close_range)
			rng.Close();

		return Error{.result = Result::Success};
	}

	void ObjData::Clear() noexcept
	{
		vertices.clear();
		texture_coordinates.clear();
		normals.clear();
		face_indices.clear();
		indices.clear();
		groups.clear();
		schema = {};
		unique_faces.clear();
	}

	void ObjData::ClearUniqueFaces() noexcept
	{
		unique_faces = {};
	}

	void ObjData::ConstructUniqueFaces() noexcept
	{
		if(face_indices.empty())
			return;

		ClearUniqueFaces();
		unique_faces = UniqueIndexMap(ObjDataFaceComparator(face_indices, schema.face_type));

		std::uint32_t id = 0;
		for(std::size_t i = 0; i < face_indices.size(); i += FaceTypeToCount(schema.face_type))
		{
			auto it = unique_faces.insert({i, 0});
			if(it.second)//inserted -> add vertex data
				it.first->second = id++;
		}
	}

	const std::vector<float> & ObjData::GetVertices() const noexcept
	{
		return vertices;
	}

	const std::vector<float> & ObjData::GetTextureCoordinates() const noexcept
	{
		return texture_coordinates;
	}

	const std::vector<float> & ObjData::GetNormals() const noexcept
	{
		return normals;
	}

	const std::vector<std::uint32_t> & ObjData::GetFaceIndices() const noexcept
	{
		return face_indices;
	}

	const std::vector<std::uint32_t> & ObjData::GetIndices() const noexcept
	{
		return indices;
	}

	const std::vector<ObjDataGroup> & ObjData::GetGroups() const noexcept
	{
		return groups;
	}

	const ObjDataSchema & ObjData::GetSchema() const noexcept
	{
		return schema;
	}

	const ObjData::UniqueIndexMap & ObjData::GetUniqueFaceIndices() const noexcept
	{
		return unique_faces;
	}

	const float * ObjData::GetVertexByIndex(std::size_t index) const noexcept
	{
		return vertices.data() + index * schema.vertex_components;
	}

	const float * ObjData::GetTextureCoordinatesByIndex(std::size_t index) const noexcept
	{
		return texture_coordinates.data() + index * schema.texture_components;
	}

	const float * ObjData::GetNormalByIndex(std::size_t index) const noexcept
	{
		return normals.data() + index * 3;
	}

	std::pair<const std::uint32_t *, std::uint32_t>
	ObjData::GetFaceIndexIdPair(std::size_t index) const noexcept
	{
		return {face_indices.data() + index * FaceTypeToCount(schema.face_type), indices[index]};
	}
};
