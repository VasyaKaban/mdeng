#pragma once

#include <map>
#include <cstdint>
#include "../../Vulkan/VulkanInclude.hpp"
#include "../../hrs/unexpected_result.hpp"

namespace FireLand
{
	enum class ShaderResult
	{
		NoRequiredShaderStage
	};

	constexpr auto ShaderResultToString(ShaderResult res) noexcept
	{
		switch(res)
		{
			case ShaderResult::NoRequiredShaderStage:
				return "NoRequiredShaderStage";
				break;
			default:
				return "";
				break;
		}
	}

	class UnexpectedShaderResult : public hrs::unexpected_result_interface
	{

	public:
		UnexpectedShaderResult(ShaderResult _result,
							   const std::source_location &_location = std::source_location::current()) noexcept
			: result(_result),
			  location(_location) {}

		~UnexpectedShaderResult() = default;
		UnexpectedShaderResult(const UnexpectedShaderResult &) = default;
		UnexpectedShaderResult(UnexpectedShaderResult &&) = default;
		UnexpectedShaderResult & operator=(const UnexpectedShaderResult &) = default;
		UnexpectedShaderResult & operator=(UnexpectedShaderResult &&) = default;

		virtual const std::source_location & GetSourceLocation() const noexcept override
		{
			return location;
		}

		virtual std::string GetErrorMessage() const override
		{
			return ShaderResultToString(result);
		}

	private:
		ShaderResult result;
		std::source_location location;
	};

	struct ShaderInfo
	{
		vk::ShaderModule shader_module;
		std::string entry_name;

		ShaderInfo(vk::ShaderModule _shader_module,
				   const std::string &_entry_name)
			: shader_module(_shader_module),
			  entry_name(_entry_name) {}

		ShaderInfo(vk::ShaderModule _shader_module,
				   std::string &&_entry_name) noexcept
			: shader_module(_shader_module),
			  entry_name(std::move(_entry_name)) {}
	};

	class Shader
	{
	public:
		virtual ~Shader() {}
		virtual void Destroy() = 0;
		virtual bool IsCreated() const noexcept = 0;
		virtual bool IsSceneIndependent() const noexcept = 0;
		virtual vk::Pipeline GetPipeline() const noexcept = 0;
		virtual vk::PipelineLayout GetPipelineLayout() const noexcept = 0;
	};
};
