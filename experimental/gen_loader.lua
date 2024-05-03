function CheckArgumentCount()
	if(#arg < 3)
	then
		error("Too few arguments!")
	end
end

function CheckLoaderType()
	if(not(arg[2] == "Instance" or arg[2] == "Device"))
	then
		error("Bad loader type: " .. arg[2])
	end
end

function GetInitLineFormatFunction(loader_type)
	format_str =
	[[

		func = get_proc_addr(%s, "%s");
		if(func)
			%s = reinterpret_cast<PFN_%s>(func);
		else
		{
			if(!std::forward<F>(check)(reinterpret_cast<PFN_vkVoidFunction>(%s), "%s"))
				return false;
		}
	]]

	return
		function(name)
			return string.format(format_str, loader_type:lower(), name, name, name, name, name)
		end
end

function CreateFieldLine(name)
	return string.format("\tPFN_%s %s;\n", name, name)
end

function GetInitArguments()
	if(arg[2] == "Instance")
	then
		return "VkInstance instance", "PFN_vkGetInstanceProcAddr get_proc_addr = vkGetInstanceProcAddr"
	else
		return "VkDevice device", "PFN_vkGetDeviceProcAddr get_proc_addr"
	end
end

CheckArgumentCount()
CheckLoaderType()


--Loader name, members, init
loader_string_format =
[[
#pragma once

#include <type_traits>
#include <string_view>

#include "VulkanInclude.h"

struct %s
{
%s

	constexpr static bool DefaultCheck(PFN_vkVoidFunction func, std::string_view) noexcept
	{
		return false;
	}

	template<typename F>
		requires std::is_invocable_r_v<bool, F, PFN_vkVoidFunction, std::string_view>
	bool Init(%s, %s, F &&check = DefaultCheck) noexcept
	{
		PFN_vkVoidFunction func;
	%s
		return true;
	}
};
]]

loader_name = arg[1]

loader_members = ""
loader_init_body = ""

init_line_format_function = GetInitLineFormatFunction(arg[2])

for i = 4, #arg
do
	loader_members = loader_members .. CreateFieldLine(arg[i])
	loader_init_body = loader_init_body .. init_line_format_function(arg[i])
end

init_arg_1, init_arg_2 = GetInitArguments()

loader_string = string.format(loader_string_format, loader_name, loader_members, init_arg_1, init_arg_2, loader_init_body)

print(loader_string)

file = io.open(arg[3], "w+")
file:write(loader_string)
file:close()
