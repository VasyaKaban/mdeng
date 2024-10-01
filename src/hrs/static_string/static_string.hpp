#pragma once

#include "../instantiation.hpp"
#include "../one_of.hpp"
#include <concepts>
#include <cstddef>
#include <iterator>

namespace hrs
{
    template<typename C, std::size_t N>
    struct static_string_data
    {
        using type = C[N];
    };

    template<typename C>
    struct static_string_data<C, 0>
    {
        using type = std::byte;
    };

    template<typename C, std::size_t N>
    using static_string_data_t = static_string_data<C, N>::type;

    template<std::size_t Start>
    struct static_string_start
    {};

    template<typename C>
    concept char_type = one_of_type<C, char, wchar_t, char8_t, char16_t, char32_t>;

    template<char_type C, std::size_t N>
    struct static_string
    {
        constexpr static std::size_t npos = N;
        constexpr static std::size_t size = N;
        using char_t = C;
        using iterator = const C*;
        using reverse_iterator = std::reverse_iterator<const C*>;
    private:
        template<char_type Ch, std::size_t M>
        friend class static_string;

        constexpr static_string() = default;
    public:
        constexpr static_string(const C (&_data)[N + 1]) noexcept
        {
            if constexpr(size != 0)
                for(std::size_t i = 0; i < N; i++)
                    data[i] = _data[i];
            else
                data = std::byte{0};
        }

        template<std::size_t M, std::size_t Start>
        requires(M >= Start + N)
        constexpr static_string(static_string<C, M> str, static_string_start<Start> start) noexcept
        {
            if constexpr(size != 0)
                for(std::size_t i = 0; i < N; i++)
                    data[i] = str.data[Start + i];
            else
                data = std::byte{0};
        }

        template<std::size_t M>
        constexpr bool operator==(const static_string<C, M>& s) const noexcept
        {
            if constexpr(size != M)
                return false;
            else if constexpr(size != 0)
            {
                for(std::size_t i = 0; i < N; i++)
                    if(data[i] != s.data[i])
                        return false;

                return true;
            }
            else
                return true;
        }

        template<std::size_t M>
        constexpr bool operator==(const C (&s)[M]) const noexcept
        {
            return operator==(static_string<C, M - 1>(s));
        }

        //template<auto str, std::size_t Start = 0>
        //	requires hrs::non_type_instantiation<decltype(str), static_string>
        template<std::size_t Start = 0, std::size_t M>
        constexpr std::size_t find(const static_string<C, M>& str) const noexcept
        {
            //constexpr std::size_t M = decltype(str)::size;
            if constexpr(M > N - Start)
                return npos;
            else
            {
                for(std::size_t i = Start; i < N; i++)
                {
                    bool found = true;
                    std::size_t k = i;
                    for(std::size_t j = 0; j < M; j++)
                    {
                        if(data[k] != str.data[j])
                            if(N - j < i)
                                return npos;
                            else
                                found = false;
                        else if(j == M - 1)
                            return i;
                        else if(k + 1 == N)
                            return npos;
                        else
                            k++;
                    }

                    if(found)
                        return i;
                }

                return npos;
            }
        }

        template<std::size_t Start = 0, std::size_t M>
        constexpr std::size_t find(const C (&s)[M]) const noexcept
        {
            return find<Start>(static_string<C, M - 1>(s));
        }

        template<std::size_t Start = 0, std::size_t M>
        constexpr iterator find_it(const static_string<C, M>& str) const noexcept
        {
            std::size_t index = find<Start>(str);
            if(index == npos)
                return end();
            else
                return iterator(data + index);
        }

        template<std::size_t Start = 0, std::size_t M>
        constexpr iterator find_it(const C (&s)[M]) const noexcept
        {
            return find_it<Start>(static_string<C, M - 1>(s));
        }

        template<std::size_t M>
        constexpr bool starts_with(const static_string<C, M>& str) const noexcept
        {
            if constexpr(M > N)
                return false;
            else
            {
                for(std::size_t i = 0; i < M; i++)
                    if(data[i] != str.data[i])
                        return false;

                return true;
            }
        }

        template<std::size_t M>
        constexpr bool starts_with(const C (&s)[M]) const noexcept
        {
            return starts_with(static_string<C, M - 1>(s));
        }

        template<std::size_t M>
        constexpr bool ends_with(const static_string<C, M>& str) const noexcept
        {
            if constexpr(M > N)
                return false;
            else
            {
                for(std::size_t i = N - M; i < N; i++)
                    if(data[i] != str.data[i])
                        return false;

                return true;
            }
        }

        template<std::size_t M>
        constexpr bool ends_with(const C (&s)[M]) const noexcept
        {
            return ends_with(static_string<C, M - 1>(s));
        }

        template<std::size_t M>
        constexpr bool contains(const static_string<C, M>& str) const noexcept
        {
            if constexpr(M > N)
                return false;
            else
                return find(str) != npos;
        }

        template<std::size_t M>
        constexpr bool contains(const C (&s)[M]) const noexcept
        {
            return contains(static_string<C, M - 1>(s));
        }

        template<std::size_t M>
        constexpr static_string<C, N + M>
        operator+(const hrs::static_string<C, M>& str) const noexcept
        {
            static_string<C, N + M> out_str;

            if constexpr(size != 0)
                for(std::size_t i = 0; i < N; i++)
                    out_str.data[i] = data[i];

            if constexpr(M != 0)
                for(std::size_t i = N; i < N + M; i++)
                    out_str.data[i] = str.data[i - N];

            return out_str;
        }

        template<std::size_t M>
        constexpr auto operator+(const C (&s)[M]) const noexcept
        {
            return operator+(static_string<C, M - 1>(s));
        }

        template<std::size_t Count>
        requires(N >= Count)
        constexpr auto substr_front() const noexcept
        {
            return substr<0, Count>();
        }

        template<std::size_t Count>
        requires(N >= Count)
        constexpr auto substr_back() const noexcept
        {
            return substr<N - Count, Count>();
        }

        template<std::size_t Start, std::size_t Count>
        requires(Start + Count <= N)
        constexpr auto substr() const noexcept
        {
            return static_string<C, Count>(*this, static_string_start<Start>{});
        }

        constexpr const C* const get_ptr() const noexcept
        {
            return data;
        }

        constexpr const char& operator[](std::size_t index) const noexcept
        {
            return data[index];
        }

        constexpr static bool is_empty() noexcept
        {
            return size == 0;
        }

        constexpr iterator begin() noexcept
        {
            if constexpr(size == 0)
                return iterator(nullptr);
            else
                return iterator(data);
        }

        constexpr iterator begin() const noexcept
        {
            if constexpr(size == 0)
                return iterator(nullptr);
            else
                return iterator(data);
        }

        constexpr iterator end() noexcept
        {
            if constexpr(size == 0)
                return iterator(nullptr);
            else
                return iterator(data + size);
        }

        constexpr iterator end() const noexcept
        {
            if constexpr(size == 0)
                return iterator(nullptr);
            else
                return iterator(data + size);
        }

        constexpr reverse_iterator rbegin() noexcept
        {
            if constexpr(size == 0)
                return reverse_iterator(iterator(nullptr));
            else
                return reverse_iterator(end());
        }

        constexpr reverse_iterator rbegin() const noexcept
        {
            if constexpr(size == 0)
                return reverse_iterator(iterator(nullptr));
            else
                return reverse_iterator(end());
        }

        constexpr reverse_iterator rend() noexcept
        {
            if constexpr(size == 0)
                return reverse_iterator(iterator(nullptr));
            else
                return reverse_iterator(begin());
        }

        constexpr reverse_iterator rend() const noexcept
        {
            if constexpr(size == 0)
                return reverse_iterator(iterator(nullptr));
            else
                return reverse_iterator(begin());
        }

        static_string_data_t<C, N> data;
    };

    template<typename C, std::size_t N>
    static_string(const C (&)[N]) -> static_string<C, N - 1>;

    /*constexpr void test_f() noexcept
	{
		auto res = split_and_exec<haystack, needle>([](auto val)
		{
			if constexpr(val.size == 0)
			{
				std::cout<<""<<std::endl;
			}
			else
			{
				std::string_view v{val.data, val.size};
				std::cout<<v<<std::endl<<std::endl;
			}

			return std::monostate{};
		});
	}*/

    /*constexpr auto spl1 = split_cursor_create<haystack, needle>();
	static_assert(spl1.value == "Hello");

	constexpr auto spl2 = spl1.next();
	static_assert(spl2.value == "");

	constexpr auto spl3 = spl2.next();
	static_assert(spl3.value == "world ");

	constexpr auto spl4 = spl3.next();
	static_assert(spl4.value == "");

	constexpr auto spl5 = spl4.next();
	static_assert(spl5.value == "0it's me!");*/
};

/*template<typename ...Classes>
constexpr auto concat_member_fields_vars(hrs::variadic<Classes...>) noexcept
{
	if constexpr(sizeof...(Classes) == 0)
		return hrs::variadic{};
	else
		return (typename class_meta<Classes>::member_fields{} + ...);
}

template<typename ...Classes>
constexpr auto concat_static_fields_vars(hrs::variadic<Classes...>) noexcept
{
	if constexpr(sizeof...(Classes) == 0)
		return hrs::variadic{};
	else
		return (typename class_meta<Classes>::static_fields{} + ...);
}

template<typename ...Classes>
constexpr auto concat_using_fields_vars(hrs::variadic<Classes...>) noexcept
{
	if constexpr(sizeof...(Classes) == 0)
		return hrs::variadic{};
	else
		return (typename class_meta<Classes>::using_fields{} + ...);
}

class Base
{
public:
	double n;
protected:
	int prot_int = 4;

	using protected_using = double;

	ALLOW_REFL_PRIVATE_ACCESS(Base)
};

struct BaseInfo
{
	bool is_protected;
	constexpr BaseInfo(bool _is_protected) noexcept : is_protected(_is_protected) {}
};


struct sql_mappable {};

REFL_BEGIN(Base)
	REFL_PARENTS_DEFAULT()
	REFL_FIELDS_BEGIN()
		REFL_MEMBER_FIELD(prot_int, sql_mappable{}),
		REFL_MEMBER_FIELD(n, sql_mappable{})
	REFL_FIELDS_END()
	REFL_STATIC_FIELDS_DEFAULT()
	REFL_USING_FIELDS_DEFAULT()
	//REFL_USINGS_BEGIN()
	//	REFL_USING_FIELD(protected_using)
	//REFL_USINGS_END()
REFL_END(Base)

template<typename C, std::size_t N>
struct SomeClassInfo
{
	hrs::static_string<C, N> str;
	constexpr SomeClassInfo(const hrs::static_string<C, N> &_str) noexcept
		: str(_str) {}
};

template<typename C, std::size_t N>
SomeClassInfo(const hrs::static_string<C, N> &_str) noexcept -> SomeClassInfo<C, N>;

struct SomeInfo
{
	std::string_view v;

};

class SomeClass : public Base
{
public:
	float public_float = 1;
	void public_foo(int);
	int any = 2;

	constexpr bool is_private_int_same(int value) const noexcept
	{
		return value == private_int;
	}
private:
	int private_int = 3;
	void private_foo(float);

	constexpr static int static_private_int = 10;

	ALLOW_REFL_PRIVATE_ACCESS(SomeClass)
};

REFL_BEGIN(SomeClass)
	REFL_PARENT_CLASSES(Base)
	REFL_FIELDS_BEGIN()
		REFL_MEMBER_FIELD(public_float, sql_mappable{}),
		REFL_MEMBER_FIELD(private_int, sql_mappable{}),
		REFL_MEMBER_FIELD(public_foo),
		REFL_MEMBER_FIELD(private_foo)
	REFL_FIELDS_END()
	REFL_STATIC_FIELDS_BEGIN()
		REFL_STATIC_FIELD(static_private_int)
	REFL_STATIC_FIELDS_END()
	REFL_USINGS_BEGIN()
		REFL_USING_FIELD(protected_using)
	REFL_USINGS_END()
	//REFL_USING_FIELDS_DEFAULT();
REFL_END(SomeClass)

template<typename C, typename F>
constexpr auto unpack_fields(F &&func) noexcept;

template<typename C, typename ...Bases, typename F>
constexpr auto unpack_fields_with_bases(hrs::variadic<Bases...>, F &&func) noexcept
{
	return std::forward<F>(func)(typename class_meta<C>::member_fields{}) + (std::forward<F>(func)(unpack_fields<Bases>(std::forward<F>(func))) + ... + hrs::variadic{});
}

template<typename C, typename F>
constexpr auto unpack_fields(F &&func) noexcept
{
	return unpack_fields_with_bases<C>(typename class_meta<C>::parents{}, std::forward<F>(func));
}

template<typename C, auto _ptr>
concept has_access = requires(C &&obj)
{
	obj.*_ptr;
};


template<hrs::type_instantiation<hrs::variadic> fields, typename C, std::size_t Index, typename ...Fields>
constexpr auto filter_sql_fields() noexcept
{
	if constexpr(Index == sizeof...(Fields))
		return fields{};
	else
	{
		using type = hrs::variadic<Fields...>::template nth_t<Index>;
		if constexpr(type::template have<sql_mappable>() && has_access<C, type::ptr>)
			return filter_sql_fields<decltype(fields{} + hrs::variadic<type>{}), C, Index + 1, Fields...>();
		else
			return filter_sql_fields<fields, C, Index + 1, Fields...>();
	}
}

template<typename C, typename ...Fields>
constexpr auto filter_sql_fields(hrs::variadic<Fields...> fields) noexcept
{
	//return fields;
	return filter_sql_fields<hrs::variadic<>, C, 0, Fields...>();
}

constexpr auto vars = unpack_fields<SomeClass>([](const auto &vars)
{
	return filter_sql_fields<SomeClass>(vars);
});

constexpr std::size_t vars_count = decltype(vars)::COUNT;

template<std::size_t Index, hrs::static_string query, typename ...Fields>
constexpr auto create_query(hrs::variadic<Fields...> fields) noexcept
{
	using fields_t = hrs::variadic<Fields...>;
	if constexpr(Index == fields_t::COUNT)
		return query;
	else
	{
		using field_t = fields_t::template nth_t<Index>;
		if constexpr(Index == 0)
			return create_query<Index + 1, field_t::name>(fields);
		else
			return create_query<Index + 1, query + hrs::static_string(", ") + field_t::name>(fields);
	}
}

template<typename C, hrs::static_string table>
constexpr auto create_select_query() noexcept
{
	constexpr auto fields = unpack_fields<C>([](const auto &vars)
	{
		return filter_sql_fields<C>(vars);
	});
	constexpr auto query = create_query<0, "">(fields);

	return
		hrs::static_string("SELECT (") +
		query +
		hrs::static_string(") FROM ") +
		table + hrs::static_string(";");
}

template<std::size_t Index, std::size_t N, hrs::static_string query>
constexpr auto create_insert_query_format_variables() noexcept
{
	if constexpr(Index == N)
		return query;
	else if constexpr(Index == 0)
		return create_insert_query_format_variables<Index + 1, N, "{}">();
	else
		return create_insert_query_format_variables<Index + 1, N, query + ", {}">();
}

template<typename C, hrs::static_string table>
constexpr auto create_insert_query_format() noexcept
{
	constexpr auto fields = unpack_fields<C>([](const auto &vars)
	{
		return filter_sql_fields<C>(vars);
	});
	constexpr auto query = create_query<0, "">(fields);

	return
		hrs::static_string("INSERT INTO ") +
		table +
		" (" +
		query +
		") VALUES(" +
		create_insert_query_format_variables<0, decltype(fields)::COUNT, "">() +
		");";
}

template<hrs::static_string fmt, typename ...Args>
auto fields_to_format_args(Args &&...args)
{
	constexpr std::string_view fmt_view{fmt.data, fmt.size};
	return std::vformat(fmt_view, std::make_format_args(args...));
}

template<hrs::static_string fmt, typename C, hrs::non_type_instantiation<member_field> ...Fields>
auto fields_to_format_fields(const C &obj, hrs::variadic<Fields...> vars)
{
	return fields_to_format_args<fmt>(Fields::get(obj)...);
}

template<hrs::static_string fmt, typename C>
auto fields_to_format(const C &obj)
{
	constexpr auto fields = unpack_fields<C>([](const auto &vars)
	{
		return filter_sql_fields<C>(vars);
	});
	return fields_to_format_fields<fmt>(obj, fields);
};

constexpr auto select_query = create_select_query<SomeClass, "SomeClassTable">();
constexpr auto insert_query_format = create_insert_query_format<SomeClass, "SomeClassTable">();

template<hrs::static_string name, std::size_t Index, hrs::non_type_instantiation<member_field> ...Fields>
constexpr auto find_member_field(hrs::variadic<Fields...> fields) noexcept
{
	static_assert(Index < decltype(fields)::COUNT, "No field with desired name!");
	using field_t = decltype(fields)::template nth_t<Index>;
	if constexpr(name == field_t::name)
		return field_t{};
	else
		return find_member_field<name, Index + 1>(fields);
}

template<hrs::static_string name, typename C>
constexpr auto find_member_field() noexcept
{
	return find_member_field<name, 0>(typename class_meta<C>::member_fields{});
}*/

/*static_assert(find_member_field<"public_float", SomeClass>().name == "public_float" &&
			  std::same_as<decltype(find_member_field<"public_float", SomeClass>())::type, float>);
static_assert(find_member_field<"public_foo", SomeClass>().name == "public_foo" &&
			  std::same_as<decltype(find_member_field<"public_foo", SomeClass>())::type, void (int)>);
static_assert(find_member_field<"private_int", SomeClass>().name == "private_int" &&
			  std::same_as<decltype(find_member_field<"private_int", SomeClass>())::type, int>);
static_assert(find_member_field<"private_foo", SomeClass>().name == "private_foo" &&
			  std::same_as<decltype(find_member_field<"private_foo", SomeClass>())::type, void (float)>);

constexpr static auto fl = find_member_field<"prot_int", SomeClass>();

constexpr static SomeClass sc{};
constexpr static auto value = decltype(fl)::get(sc);
constexpr static auto value2 = decltype(find_member_field<"private_int", SomeClass>())::get(sc);

constexpr auto change_private_member(int value = 10)
{
	SomeClass sc;
	decltype(find_member_field<"private_int", SomeClass>())::get(sc) = value;
	return sc;
}
*/
//template<hrs::static_string name, std::size_t Index, /*hrs::non_type_instantiation<static_field>*/ typename ...Fields>
/*constexpr auto find_static_field(hrs::variadic<Fields...> fields) noexcept
{
	static_assert(Index < decltype(fields)::COUNT, "No field with desired name!");
	using field_t = decltype(fields)::template nth_t<Index>;
	if constexpr(name == field_t::name)
		return field_t{};
	else
		return find_static_field<name, Index + 1>(fields);
}

template<hrs::static_string name, typename C>
constexpr auto find_static_field() noexcept
{
	return find_static_field<name, 0>(typename class_meta<C>::static_fields{});
}

constexpr static auto static_fl = find_static_field<"static_private_int", SomeClass>();
static_assert(class_meta<decltype(static_fl)::class_type>::name == "SomeClass");
constexpr static auto static_fl_value = static_fl.get();
*/
