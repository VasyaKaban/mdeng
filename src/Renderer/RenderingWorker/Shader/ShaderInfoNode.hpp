#pragma once

#include "Shader.h"
#include "../../../hrs/non_creatable.hpp"

namespace FireLand
{
	template<std::derived_from<Shader> S>
	struct ShaderInfoNode : public hrs::non_copyable
	{
		std::map<vk::ShaderStageFlagBits, ShaderInfo> shader_modules;
		std::unique_ptr<S> shader;

		~ShaderInfoNode() = default;
		ShaderInfoNode(const std::map<vk::ShaderStageFlagBits, ShaderInfo> &_shader_modules,
					   S *_shader)
			: shader_modules(_shader_modules),
			  shader(_shader) {}

		ShaderInfoNode(std::map<vk::ShaderStageFlagBits, ShaderInfo> &&_shader_modules,
					   S *_shader)
			: shader_modules(std::move(_shader_modules)),
			  shader(_shader) {}

		ShaderInfoNode(ShaderInfoNode &&) = default;
		ShaderInfoNode & operator=(ShaderInfoNode &&) = default;
	};
};
