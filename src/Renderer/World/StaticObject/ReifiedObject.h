#pragma once

#include <span>
#include <cstddef>
#include <cstdint>

namespace FireLand
{
	namespace StaticObject
	{
		class ReifiedObject
		{
		public:
			virtual ~ReifiedObject() {}
			virtual std::size_t GetMeshCount() const noexcept = 0;
			virtual std::span<const std::byte> GetMeshVertexData() const noexcept = 0;
			virtual std::span<const std::uint32_t> GetMeshIndexData() const noexcept = 0;
			virtual std::size_t GetMeshIndexDataOffset(std::size_t index) const noexcept = 0;
			virtual std::uint32_t GetMeshIndicesCount(std::size_t index) const noexcept = 0;
		};
	};
};
