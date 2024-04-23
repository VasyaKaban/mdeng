#include "ObjParserRange.h"
#include <ranges>

namespace GeometryParser
{
	ObjParserRange::~ObjParserRange()
	{
		Close();
	}

	ObjParserRange::ObjParserRange(ObjParserRange &&rng) noexcept
		: stream(std::move(rng.stream)),
		  schema(std::move(rng.schema)),
		  line(std::move(rng.line)),
		  face_indices(std::move(rng.face_indices)) {}

	ObjParserRange & ObjParserRange::operator=(ObjParserRange &&rng) noexcept
	{
		Close();

		stream = std::move(rng.stream);
		schema = std::move(rng.schema);
		line = std::move(rng.line);
		face_indices = std::move(rng.face_indices);

		return *this;
	}

	Result ObjParserRange::Open(const std::filesystem::path &path, bool read_all) noexcept
	{
		Close();
		schema = {};

		if(!std::holds_alternative<std::ifstream>(stream))
			stream = std::ifstream{};

		auto &file_stream = std::get<std::ifstream>(stream);
		file_stream.open(path);
		if(!file_stream.is_open())
			return Result::BadFile;

		if(read_all)
		{
			std::error_code code;
			auto file_size = std::filesystem::file_size(path, code);
			if(code == std::errc(0))
			{
				std::string data;
				data.resize(file_size);
				file_stream.read(data.data(), file_size);
				return Consume(std::move(data));
			}
		}

		return Result::Success;
	}

	Result ObjParserRange::Consume(const std::string &data)
	{
		Close();
		schema = {};

		if(!std::holds_alternative<std::istringstream>(stream))
			stream = std::istringstream{};

		std::get<std::istringstream>(stream).str(data);

		return Result::Success;
	}

	Result ObjParserRange::Consume(std::string &&data) noexcept
	{
		Close();
		schema = {};

		if(!std::holds_alternative<std::istringstream>(stream))
			stream = std::istringstream{};

		std::get<std::istringstream>(stream).str(std::move(data));

		return Result::Success;
	}

	void ObjParserRange::Close() noexcept
	{
		if(std::holds_alternative<std::ifstream>(stream))
		{
			auto &file_stream = std::get<std::ifstream>(stream);
			if(IsOpen())
				file_stream.close();
		}
	}

	bool ObjParserRange::IsOpen() const noexcept
	{
		if(std::holds_alternative<std::ifstream>(stream))
			return std::get<std::ifstream>(stream).is_open();

		return true;
	}

	hrs::expected<ElementData, Error> ObjParserRange::Next()
	{
		auto create_error = [](const std::string &line, const parse_result &res)
		{
			return Error{.result = res.result,
						 .str = line,
						 .column = static_cast<size_t>(res.ptr - line.data())};
		};

		auto read_line = [&]() -> bool
		{
			if(std::holds_alternative<std::ifstream>(stream))
			{
				auto &file_stream = std::get<std::ifstream>(stream);
				std::getline(file_stream, line);
				return file_stream.eof();
			}
			else
			{
				auto &str_stream = std::get<std::istringstream>(stream);
				std::getline(str_stream, line);
				return str_stream.eof();
			}
		};

		if(!IsOpen())
			return Error{.result = Result::BadFile};

		while(true)
		{
			bool eof = read_line();
			if(eof)
				return Error{.result = Result::EndOfFile};

			if(line.starts_with("v "))
			{
				auto exp = parse_vertex(line);
				if(!exp)
					return create_error(line, exp.error());

				return ElementData{.tag = schema.GetVertexElement(), .data = exp.value()};
			}
			else if(line.starts_with("vt "))
			{
				auto exp = parse_texture_coordinates(line);
				if(!exp)
					return create_error(line, exp.error());

				return ElementData{.tag = schema.GetTextureCoordinatesElement(), .data = exp.value()};
			}
			else if(line.starts_with("vn "))
			{
				auto exp = parse_normal(line);
				if(!exp)
					return create_error(line, exp.error());

				return ElementData{.tag = Element::Normal, .data = exp.value()};
			}
			else if(line.starts_with("f "))
			{
				auto exp = parse_face(line);
				if(!exp)
					return create_error(line, exp.error());

				return ElementData{.tag = schema.FaceTypeElement(), .data = exp.value()};
			}
			else if(line.starts_with("g "))
			{
				auto exp = parse_group(line);
				if(!exp)
					return create_error(line, exp.error());

				return ElementData{.tag = Element::Group, .data = exp.value()};
			}
		}

		return Error{.result = Result::EndOfFile};
	}

	const ObjParserSchema & ObjParserRange::GetSchema() const noexcept
	{
		return schema;
	}

	std::string_view ObjParserRange::trim_spaces_back(std::string_view str) const noexcept
	{
		for(auto rit = str.rbegin(); rit != str.rend(); rit++)
		{
			if(!(*rit.base() == ' ' || *rit.base() == '\t'))
			{
				str = std::string_view(str.begin(), rit.base());
				break;
			}
		}

		return str;
	}

	hrs::expected<VectorElement, ObjParserRange::parse_result>
	ObjParserRange::parse_vector_element(std::string_view str,
										 std::uint8_t &schema_components,
										 std::uint8_t min_components,
										 std::uint8_t max_components,
										 Result bad_result) noexcept
	{
		auto trimmed = trim_spaces_back(str);
		if(trimmed.empty())
			return parse_result{.result = bad_result, .ptr = str.data()};

		VectorElement elem;
		if(schema_components == 0)//no schema
		{
			std::uint8_t i = 0;
			for(const auto value : std::ranges::split_view(trimmed, ' ') | std::views::drop(1))
			{
				if((i + 1) > max_components)
					return parse_result{.result = bad_result, .ptr = value.data()};

				auto float_opt = parse_value<float>(std::string_view(value.begin(), value.end()));
				if(!float_opt)
					return parse_result{.result = bad_result, .ptr = value.data()};

				elem.data[i] = *float_opt;
				i++;
			}

			if(i < min_components)
				return parse_result{.result = bad_result, .ptr = trimmed.data()};

			schema_components = i;
		}
		else
		{
			std::uint8_t i = 0;
			for(const auto value : std::ranges::split_view(trimmed, ' ') | std::views::drop(1))
			{
				if((i + 1) > schema_components)
					return parse_result{.result = bad_result, .ptr = value.data()};

				auto float_opt = parse_value<float>(std::string_view(value.begin(), value.end()));
				if(!float_opt)
					return parse_result{.result = bad_result, .ptr = value.data()};

				elem.data[i] = *float_opt;
				i++;
			}

			if(i != schema_components)
				return parse_result{.result = Result::BadVertex, .ptr = trimmed.data()};
		}

		return elem;
	}

	ObjParserRange::face_parse_slashes
	ObjParserRange::find_face_slashes(std::string_view str) const noexcept
	{
		face_parse_slashes out{.first = std::string_view::npos, .second = std::string_view::npos};
		out.first  = str.find('/');
		if(out.first == std::string_view::npos)
			return out;

		out.second = str.find('/', out.first + 1);
		return out;
	}

	ObjParserRange::parse_result
	ObjParserRange::parse_face_element(std::string_view str, const face_parse_slashes &slashes) noexcept
	{
		switch(schema.face_type)
		{
			case FaceType::Vertex:
				{
					auto value_opt = parse_value<std::uint32_t>(str);
					if(!value_opt)
						return parse_result{.result = Result::BadFace, .ptr = str.data()};

					face_indices.push_back(*value_opt);
				}
				break;
			case FaceType::VertexTextureCoordinates:
				{
					std::string_view values[2] =
					{
						std::string_view(str.begin(), str.begin() + slashes.first),
						std::string_view(str.begin() + slashes.first + 1, str.end())
					};

					for(const auto &val : values)
					{
						auto value_opt = parse_value<std::uint32_t>(val);
						if(!value_opt)
							return parse_result{.result = Result::BadFace, .ptr = str.data()};

						face_indices.push_back(*value_opt);
					}
				}
				break;
			case FaceType::VertexNormal:
				{
					std::string_view values[2] =
					{
						std::string_view(str.begin(), str.begin() + slashes.first),
						std::string_view(str.begin() + slashes.second + 1, str.end())
					};

					for(const auto &val : values)
					{
						auto value_opt = parse_value<std::uint32_t>(val);
						if(!value_opt)
							return parse_result{.result = Result::BadFace, .ptr = str.data()};

						face_indices.push_back(*value_opt);
					}
				}
				break;
			case FaceType::VertexTextureCoordinatesNormal:
				{
					std::string_view values[3] =
					{
						std::string_view(str.begin(), str.begin() + slashes.first),
						std::string_view(str.begin() + slashes.first + 1, str.begin() + slashes.second),
						std::string_view(str.begin() + slashes.second + 1, str.end())
					};

					for(const auto &val : values)
					{
						auto value_opt = parse_value<std::uint32_t>(val);
						if(!value_opt)
							return parse_result{.result = Result::BadFace, .ptr = str.data()};

						face_indices.push_back(*value_opt);
					}
				}
				break;
		}

		return parse_result{.result = Result::Success};
	}

	hrs::expected<VectorElement, ObjParserRange::parse_result>
	ObjParserRange::parse_vertex(std::string_view str) noexcept
	{
		return parse_vector_element(str,
									schema.vertex_components,
									3,
									4,
									Result::BadVertex);
	}

	hrs::expected<VectorElement, ObjParserRange::parse_result>
	ObjParserRange::parse_texture_coordinates(std::string_view str) noexcept
	{
		return parse_vector_element(str,
									schema.texture_coordinates_components,
									1,
									3,
									Result::BadTextureCoordinates);
	}

	hrs::expected<VectorElement, ObjParserRange::parse_result>
	ObjParserRange::parse_normal(std::string_view str) noexcept
	{
		std::uint8_t schema_normals(3);
		return parse_vector_element(str,
									schema_normals,
									3,
									3,
									Result::BadNormal);
	}

	hrs::expected<FaceElement, ObjParserRange::parse_result>
	ObjParserRange::parse_face(std::string_view str) noexcept
	{
		auto trimmed = trim_spaces_back(str);
		if(trimmed.empty())
			return parse_result{.result = Result::BadFace, .ptr = str.data()};

		face_indices.clear();
		if(schema.face_count == 0)//no schema
		{
			for(const auto face_line : std::ranges::split_view(trimmed, ' ') | std::views::drop(1))
			{
				if((schema.face_count + 1) == 255)
					return parse_result{.result = Result::BadFace, .ptr = face_line.data()};

				auto slashes = find_face_slashes(std::string_view(face_line.begin(),
																  face_line.end()));

				if(schema.face_count == 0)
					schema.face_type = slashes.GetFaceType();
				else
				{
					bool satisfy = slashes.IsSatisfy(schema.face_type);
					if(!satisfy)
						return parse_result{.result = Result::BadFace, .ptr = face_line.data()};
				}

				auto res = parse_face_element(std::string_view(face_line.begin(), face_line.end()),
											  slashes);

				if(res.result != Result::Success)
					return res;

				schema.face_count++;
			}

			if(schema.face_count < 3)
				return parse_result{.result = Result::BadFace, .ptr = trimmed.data()};
		}
		else
		{
			std::uint8_t i = 0;
			for(const auto face_line : std::ranges::split_view(trimmed, ' ') | std::views::drop(1))
			{
				if((i + 1) > schema.face_count)
					return parse_result{.result = Result::BadFace, .ptr = face_line.data()};

				auto slashes = find_face_slashes(std::string_view(face_line.begin(), face_line.end()));

				bool satisfy = slashes.IsSatisfy(schema.face_type);
				if(!satisfy)
					return parse_result{.result = Result::BadFace, .ptr = face_line.data()};

				auto res = parse_face_element(std::string_view(face_line.begin(), face_line.end()),
											  slashes);

				if(res.result != Result::Success)
					return res;

				i++;
			}

			if(i != schema.face_count)
				return parse_result{.result = Result::BadFace, .ptr = trimmed.data()};
		}

		return FaceElement(face_indices);
	}

	hrs::expected<GroupElement, ObjParserRange::parse_result>
	ObjParserRange::parse_group(std::string_view str) noexcept
	{
		auto str_plit = std::string_view(str.begin() + str.find(' ') + 1, str.end());
		auto trimmed = trim_spaces_back(str);
		if(trimmed.empty())
			return parse_result{.result = Result::BadGroup, .ptr = str.data()};

		return GroupElement(trimmed);
	}
};
