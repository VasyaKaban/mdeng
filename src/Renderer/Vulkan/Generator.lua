local GENERATOR = {}

GENERATOR["CheckType"] =
function(module_type, allowed_types)
	found = false;
	for _, name in ipairs(allowed_types)
	do
		if(name == loader_type)
		then
			found = true
			break
		end
	end

	if(not found)
	then
		error("Bad module type: " .. module_type)
	end
end

GENERATOR["CreateCondCompilationLine"] =
function(dependencies)
	if(#dependencies == 0)
	then
		return ""
	end

	local COND_COMPILE_STR = "#if "
	for i, deps in ipairs(dependencies)
	do
		if(i ~= 1)
		then
			COND_COMPILE_STR = COND_COMPILE_STR .. " || "
		end

		for j, dep in ipairs(deps)
		do
			if(j ~= 1)
			then
				COND_COMPILE_STR = COND_COMPILE_STR .. " && "
			end

			COND_COMPILE_STR = COND_COMPILE_STR .. "defined(" .. dep .. ")"

		end
	end

	return COND_COMPILE_STR .. "\n"
end

GENERATOR["CreateInstanceModule"] =
function(module_path_to_vulkan_include, module_out_path, module_namespace, module_name, module_members)
	local DECL_FORMAT =
[[
#pragma once

#include "%sVulkanInclude.h"

namespace %s
{
	struct %s
	{
%s

		unsigned int Init(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept;
	};
};
]]

	local DEF_FORMAT =
[[
#include "%s.h"

namespace %s
{
	unsigned int %s::Init(VkInstance instance, PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept
	{
		unsigned int initialized = 0;

%s
		return initialized;
	}
};
]]
	local DECL_MEMBERS = ""
	local DEF_MEMBERS = ""

	for member, dependencies in pairs(module_members)
	do
		local COND_COMPILE_LINE = GENERATOR.CreateCondCompilationLine(dependencies)
		if(COND_COMPILE_LINE ~= "")
		then
			DECL_MEMBERS = DECL_MEMBERS .. COND_COMPILE_LINE
			DEF_MEMBERS = DEF_MEMBERS .. COND_COMPILE_LINE
		end


		DECL_MEMBERS = DECL_MEMBERS .. string.format("\t\tPFN_%s %s = nullptr;\n", member, member)
		DEF_MEMBERS = DEF_MEMBERS .. string.format("\t\tif(%s = reinterpret_cast<PFN_%s>(vkGetInstanceProcAddr(instance, \"%s\")); %s != nullptr)\n\t\t\tinitialized++;\n", member, member, member, member)

		if(COND_COMPILE_LINE ~= "")
		then
			DECL_MEMBERS = DECL_MEMBERS .. "#endif\n"
			DEF_MEMBERS = DEF_MEMBERS .. "#endif\n"
		end
	end

	local DECL_DATA = string.format(DECL_FORMAT, module_path_to_vulkan_include, module_namespace, module_name, DECL_MEMBERS)
	local DEF_DATA = string.format(DEF_FORMAT, module_name, module_namespace, module_name, DEF_MEMBERS)


	GENERATOR.Write(module_out_path .. module_name .. ".h", DECL_DATA, module_out_path .. module_name .. ".cpp", DEF_DATA)

end

GENERATOR["Write"] =
function(decl_path, decl_data, def_path, def_data)
	file = io.open(decl_path, "w+")
	file:write(decl_data)
	file:close()

	file = io.open(def_path, "w+")
	file:write(def_data)
	file:close()
end

GENERATOR.CreateInstanceModule("", "", "FireLand", "TestInstanceModule",
{
	vkCreateDevice =  {},
	vkDestroySurfaceKHR = {{"VK_KHR_surface"}, {"Nigger", "Skibidi"}},
	vkCreateSwapchainKHR = {{"VK_KHR_swapchain"}}
})
