local bit = require("bit")

local quals =
{
	{name = "", value = 0},
	{name = "const", value = 1, traits_field = "\n\t\tconstexpr static bool is_const_qualified = "},
	{name = "volatile", value = 2, traits_field = "\n\t\tconstexpr static bool is_volatile_qualified = "},
	{name = "&", value = 4, conflicts = 8, traits_field = "\n\t\tconstexpr static bool is_lvalue_ref_qualified = "},
	{name = "&&", value = 8, conflicts = 4, traits_field = "\n\t\tconstexpr static bool is_rvalue_ref_qualified = "},
	{name = "noexcept", value = 16, traits_field = "\n\t\tconstexpr static bool is_noexcept_qualified = "}--2,4
	--2,5 - 1
	--11111
}

local function_signature_format = "R (%s) %s"

local trait_specialization_format =
[[
	template<typename R, typename ...Args>
	struct function_traits<%s>
	{
		using return_type = R;
		using arguments = variadic<Args...>;%s
	};
]]

local function_signatures = {}

function insert_function_signature(has_variadic, mask, file)
	local appended_quals = {}
	local quals_traits_fields = {}
	for qual_index, qual in ipairs(quals)
	do
		local has_qual = false
		if bit.band(mask, qual.value) ~= 0 or qual.value == 0 then
			if qual.conflicts ~= nil then
				if bit.band(mask, qual.conflicts) ~= 0 then
					return
				end
			end

			has_qual = true
			appended_quals[#appended_quals + 1] = qual.name
		end

		if qual.traits_field ~= nil then
			quals_traits_fields[#quals_traits_fields + 1] = qual.traits_field .. tostring(has_qual) .. ";"
		end
	end

	local quals_str = ""
	for i = 1, #appended_quals
	do
		if #quals_str == 0 then
			quals_str = appended_quals[i]
		else
			quals_str = quals_str .. " " .. appended_quals[i]
		end
	end

	local traits_fields_str = ""
	for traits_field_index, traits_field in ipairs(quals_traits_fields)
	do
		traits_fields_str = traits_fields_str .. traits_field
	end

	local args_str = "Args ..."
	if has_variadic then
		traits_fields_str = traits_fields_str .. "\n\t\tconstexpr static bool has_variadic_arguments = true;"
		args_str = args_str .. ", ..."
	else
		traits_fields_str = traits_fields_str .. "\n\t\tconstexpr static bool has_variadic_arguments = false;"
	end

	function_signatures[#function_signatures + 1] = string.format(function_signature_format, args_str, quals_str)
	local trait_specialization = string.format(trait_specialization_format, function_signatures[#function_signatures], traits_fields_str)
	print(trait_specialization)
	file:write(trait_specialization .. "\n")
end

local file = io.open("../function_traits.hpp", "w+")
file:write(
[[
#pragma once

#include <type_traits>
#include "variadic.hpp"

namespace hrs
{
	template<typename F>
		requires std::is_function_v<F>
	struct function_traits;

]])

local has_varaidic_args = {false, true}
for arg_index, has_variadic in ipairs(has_varaidic_args)
do
	for i = 0, 2 ^ (#quals - 1) - 1
	do
		insert_function_signature(has_variadic, i, file)
	end
end

file:write(
[[

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_const_qualified = function_traits<F>::is_const_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_volatile_qualified = function_traits<F>::is_volatile_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_lvalue_ref_qualified = function_traits<F>::is_lvalue_ref_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_rvalue_ref_qualified = function_traits<F>::is_rvalue_ref_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_noexcept_qualified = function_traits<F>::is_noexcept_qualified;

	template<typename F>
		requires std::is_function_v<F>
	constexpr inline bool is_function_has_variadic_arguments = function_traits<F>::has_variadic_arguments;

	template<typename F>
		requires std::is_function_v<F>
	using function_arguments = function_traits<F>::arguments;
}
]]
)

file:close()
