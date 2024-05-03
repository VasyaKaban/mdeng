#pragma once

#include <string_view>

namespace hrs
{
	namespace detail
	{
		constexpr std::string_view trim_spaces(std::string_view str) noexcept
		{
			for(std::size_t i = 0; i < str.size(); i++)
			{
				if(str[i] != ' ')
				{
					str = std::string_view(str.data() + i, str.size() - i);
					break;
				}
			}

			for(std::string_view::size_type i = str.size() - 1; i != 0; i--)
			{
				if(str[i] != ' ')
				{
					str = std::string_view(str.data(), i + 1);
					break;
				}
			}

			return str;
		}

		template<std::size_t N>
		struct token_count
		{
			constexpr static std::size_t value = (N == 0 ? 1 : N + 1);
		};

		template<std::size_t N>
		constexpr inline std::size_t token_count_v = token_count<N>::value;
	};

	template<typename E> \
		requires std::is_enum_v<E>
	struct enum_meta {};
};

#define HRS_TOKENIZE(...) \
	[]() noexcept \
	{ \
		constexpr std::string_view str = #__VA_ARGS__; \
		static_assert(!str.empty()); \
		std::array<std::string_view, hrs::detail::token_count_v<std::ranges::count(str, ',')>> tokens; \
		std::size_t i = 0; \
		for(const auto &word : std::ranges::split_view(str, ',')) \
		{ \
			tokens[i] = hrs::detail::trim_spaces(std::string_view(word.begin(), word.end())); \
			i++; \
		} \
 \
		return tokens; \
	}()

#define HRS_DETAIL_CREATE_ENUM_NAME_PAIRS(NAME, ...) \
	[]() noexcept \
	{ \
		constexpr auto names = HRS_TOKENIZE(__VA_ARGS__); \
		using enum NAME; \
		using Pair = std::pair<NAME, std::string_view>; \
		constexpr std::array<NAME, names.size()> values = {__VA_ARGS__}; \
		std::array<Pair, names.size()> pairs; \
		for(std::size_t i = 0; i < names.size(); i++) \
		pairs[i] = {values[i], names[i]}; \
\
		std::ranges::sort(pairs, [](const Pair &p1, const Pair &p2) \
		{ \
			return p1.first < p2.first; \
		}); \
\
		return pairs; \
	}()

#define HRS_GEN_ENUM_META(NAME, ...) \
	template<> \
	struct hrs::enum_meta<NAME> \
	{ \
		constexpr static auto pairs = HRS_DETAIL_CREATE_ENUM_NAME_PAIRS(NAME, __VA_ARGS__); \
		constexpr static std::size_t count = pairs.size(); \
		using underlying = std::underlying_type_t<NAME>; \
 \
		constexpr static std::string_view get_name(NAME value) noexcept \
		{ \
			auto it = std::ranges::lower_bound(pairs, std::pair<NAME, std::string_view>{value, ""}); \
			if(it == pairs.end()) \
				return {}; \
\
			return it->second; \
		} \
\
	};

#define HRS_ENUM(NAME, ...) \
	enum class NAME \
	{ \
		__VA_ARGS__ \
	}; \
\
	HRS_GEN_ENUM_META(NAME, __VA_ARGS__)

#define HRS_ENUM_TYPED(NAME, TYPE, ...) \
	enum class NAME : TYPE \
	{ \
		 __VA_ARGS__ \
	}; \
\
	HRS_GEN_ENUM_META(NAME, __VA_ARGS__)

#define HRS_ENUM_NS(NAME, NAMESPACE, ...) \
	namespace NAMESPACE \
	{ \
		enum class NAME \
		{ \
			__VA_ARGS__ \
		}; \
	}; \
\
	HRS_GEN_ENUM_META(NAMESPACE::NAME, __VA_ARGS__)

#define HRS_ENUM_NS_TYPED(NAME, NAMESPACE, TYPE, ...) \
	namespace NAMESPACE\
	{ \
		enum class NAME : TYPE \
		{ \
			__VA_ARGS__ \
		}; \
	}; \
\
	HRS_GEN_ENUM_META(NAMESPACE::NAME, __VA_ARGS__)
