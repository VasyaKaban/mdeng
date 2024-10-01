#pragma once

#include "ObjSchema.h"
#include "hrs/expected.hpp"
#include "hrs/flags.hpp"
#include "hrs/non_creatable.hpp"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <functional>
#include <optional>

namespace GeometryParser
{
    class ObjParserRange : public hrs::non_copyable
    {
    public:
        ObjParserRange() = default;
        ~ObjParserRange();
        ObjParserRange(ObjParserRange&& rng) noexcept;
        ObjParserRange& operator=(ObjParserRange&& rng) noexcept;

        Result Open(const std::filesystem::path& path, bool read_all) noexcept;
        Result Consume(const std::string& data);
        Result Consume(std::string&& data) noexcept;

        void Close() noexcept;

        bool IsOpen() const noexcept;

        hrs::expected<ElementData, Error> Next();

        const ObjParserSchema& GetSchema() const noexcept;
    private:
        struct parse_result
        {
            Result result;
            const char* ptr;
        };

        struct face_parse_slashes
        {
            std::string_view::size_type first;
            std::string_view::size_type second;

            bool IsSatisfy(FaceType face_type) const noexcept
            {
                if(first == std::string_view::npos)
                    return face_type == FaceType::Vertex;

                if(second == std::string_view::npos)
                    return face_type == FaceType::VertexTextureCoordinates;

                if(second == first + 1)
                    return face_type == FaceType::VertexNormal;

                return face_type == FaceType::VertexTextureCoordinatesNormal;
            }

            FaceType GetFaceType() const noexcept
            {
                if(first == std::string_view::npos)
                    return FaceType::Vertex;

                if(second == std::string_view::npos)
                    return FaceType::VertexTextureCoordinates;

                if(second == first + 1)
                    return FaceType::VertexNormal;

                return FaceType::VertexTextureCoordinatesNormal;
            }
        };

        template<typename T>
        std::optional<T> parse_value(std::string_view str) const noexcept;

        std::string_view trim_spaces_back(std::string_view str) const noexcept;

        hrs::expected<VectorElement, parse_result>
        parse_vector_element(std::string_view str,
                             std::uint8_t& schema_components,
                             std::uint8_t min_components,
                             std::uint8_t max_components,
                             Result bad_result) noexcept;

        face_parse_slashes find_face_slashes(std::string_view str) const noexcept;
        parse_result parse_face_element(std::string_view str,
                                        const face_parse_slashes& slashes) noexcept;

        hrs::expected<VectorElement, parse_result> parse_vertex(std::string_view str) noexcept;
        hrs::expected<VectorElement, parse_result>
        parse_texture_coordinates(std::string_view str) noexcept;
        hrs::expected<VectorElement, parse_result> parse_normal(std::string_view str) noexcept;
        hrs::expected<FaceElement, parse_result> parse_face(std::string_view str) noexcept;
        hrs::expected<GroupElement, parse_result> parse_group(std::string_view str) noexcept;
    private:
        std::variant<std::ifstream, std::istringstream> stream;
        ObjParserSchema schema;
        std::string line;
        std::vector<std::uint32_t> face_indices;
    };

    template<typename T>
    std::optional<T> ObjParserRange::parse_value(std::string_view str) const noexcept
    {
        T value;
        std::from_chars_result res = std::from_chars(str.data(), str.data() + str.size(), value);
        if(!(res.ec == std::errc(0) && res.ptr == (str.data() + str.size())))
            return {};

        return value;
    }
};
