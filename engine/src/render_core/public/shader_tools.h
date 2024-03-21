#pragma once

#include "rhi/engine_rhi.h"
#include <unordered_set>

namespace ad_astris::rcore
{
	struct ShaderInputDesc
	{
		io::URI shaderPath;
		rhi::ShaderType type{ rhi::ShaderType::UNDEFINED };
		rhi::ShaderFormat format{ rhi::ShaderFormat::UNDEFINED };
		rhi::HLSLShaderModel minHlslShaderModel{ rhi::HLSLShaderModel::SM_6_0 };
		std::string entryPoint{ "main" };
		std::vector<std::string> defines;
		std::vector<std::string> includePaths;
	};

	struct ShaderOutputDesc
	{
		std::shared_ptr<void> internalBlob;
		const uint8_t* data{ nullptr };
		uint64_t dataSize;
		std::unordered_set<std::string> dependencies;
	};

	enum class ShaderCacheType
	{
		DXIL,
		SPIRV
	};

	struct ShaderCompilerInitContext
	{
		ShaderCacheType cacheType;
	};

	class IShaderCompiler
	{
		public:
		virtual void compile(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc) = 0;
	};

	struct ShaderManagerInitContext
	{
		ShaderCacheType cacheType;
		rhi::RHI* rhi;
	};

	class IShaderManager
	{
		public:
			virtual ~IShaderManager() { }
			
			virtual void init(ShaderManagerInitContext& shaderManagerInitContext) = 0;
			virtual rhi::Shader* load_shader(
				const io::URI& relativeShaderPath,
				rhi::ShaderType shaderType,
				bool isEngineShader = true,
				rhi::HLSLShaderModel shaderModel = rhi::HLSLShaderModel::SM_6_0,
				const std::vector<std::string>& shaderDefines = {}) = 0;
			virtual rhi::Shader* get_shader(const io::URI& relativeShaderPath) = 0;
			virtual IShaderCompiler* get_shader_compiler() = 0;
	};
}