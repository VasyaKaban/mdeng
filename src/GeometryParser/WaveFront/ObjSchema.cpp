#include "ObjSchema.h"

namespace GeometryParser
{
    bool FaceElement::operator==(const FaceElement& fe) const noexcept
    {
        return data.get().data() == fe.data.get().data();
    }

    GroupElement::GroupElement(std::string_view _data)
        : data(_data)
    {}

    VectorElement& ElementData::GetVectorElement() noexcept
    {
        return std::get<VectorElement>(data);
    }

    const VectorElement& ElementData::GetVectorElement() const noexcept
    {
        return std::get<VectorElement>(data);
    }

    FaceElement& ElementData::GetFaceElement() noexcept
    {
        return std::get<FaceElement>(data);
    }

    const FaceElement& ElementData::GetFaceElement() const noexcept
    {
        return std::get<FaceElement>(data);
    }

    GroupElement& ElementData::GetGroupElement() noexcept
    {
        return std::get<GroupElement>(data);
    }

    const GroupElement& ElementData::GetGroupElement() const noexcept
    {
        return std::get<GroupElement>(data);
    }

    std::uint8_t FaceTypeToCount(FaceType type) noexcept
    {
        std::uint8_t count;
        switch(type)
        {
            case FaceType::Vertex:
                count = 1;
                break;
            case FaceType::VertexTextureCoordinates:
            case FaceType::VertexNormal:
                count = 2;
                break;
            case FaceType::VertexTextureCoordinatesNormal:
                count = 3;
                break;
        }

        return count;
    }

    Element ObjParserSchema::GetVertexElement() const noexcept
    {
        Element elem;
        switch(vertex_components)
        {
            case 3:
                elem = Element::VertexXYZ;
                break;
            case 4:
                elem = Element::VertexXYZW;
                break;
        }

        return elem;
    }

    Element ObjParserSchema::GetTextureCoordinatesElement() const noexcept
    {
        Element elem;
        switch(texture_coordinates_components)
        {
            case 1:
                elem = Element::TextureCoordinatesU;
                break;
            case 2:
                elem = Element::TextureCoordinatesUV;
                break;
            case 3:
                elem = Element::TextureCoordinatesUVW;
                break;
        }

        return elem;
    }

    Element ObjParserSchema::FaceTypeElement() const noexcept
    {
        Element elem;
        switch(face_type)
        {
            case FaceType::Vertex:
                elem = Element::FaceV;
                break;
            case FaceType::VertexTextureCoordinates:
                elem = Element::FaceVT;
                break;
            case FaceType::VertexNormal:
                elem = Element::FaceVN;
                break;
            case FaceType::VertexTextureCoordinatesNormal:
                elem = Element::FaceVTN;
                break;
        }

        return elem;
    }

    std::size_t ObjParserSchema::IndicesPerFace() const noexcept
    {
        return face_count * FaceTypeToCount(face_type);
    }
};
