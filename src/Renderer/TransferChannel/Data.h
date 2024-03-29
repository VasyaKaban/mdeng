#pragma once

#include <cstddef>

namespace FireLand
{
	class Data
	{
	public:
		Data(const std::byte *_data = {}) noexcept;

		template<typename T>
		Data(T *_data = {}) noexcept
			: data(reinterpret_cast<const std::byte *>(_data)) {}

		~Data() = default;
		Data(const Data &d) = default;
		Data(Data &&d) = default;
		Data & operator=(const Data &) = default;
		Data & operator=(Data &&d) = default;

		bool IsNonNullData() const noexcept;

		template<typename T = std::byte>
		constexpr const T * GetData() const noexcept
		{
			return reinterpret_cast<const T *>(data);
		}

	private:
		const std::byte *data;
	};
};

