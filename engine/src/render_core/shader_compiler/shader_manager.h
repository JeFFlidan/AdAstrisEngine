#pragma once

#include "render_core/api.h"
#include "shader_cache.h"
#include "engine/render_core_module.h"
#include <unordered_map>

namespace ad_astris::rcore::impl
{
	class RENDER_CORE_API ShaderManager : public IShaderManager
	{
		public:
			virtual void init(ShaderManagerInitContext& shaderManagerInitContext) override;
			virtual rhi::Shader* load_shader(
				const io::URI& relativeShaderPath,
				rhi::ShaderType shaderType,
				bool isEngineShader = true,
				rhi::HLSLShaderModel minHlslShaderModel = rhi::HLSLShaderModel::SM_6_0,
				const std::vector<std::string>& shaderDefines = {}) override;
			virtual rhi::Shader* get_shader(const io::URI& relativeShaderPath) override;
		
			virtual IShaderCompiler* get_shader_compiler() override
			{
				return _shaderCompiler.get();
			}
		
		private:
			rhi::RHI* _rhi{ nullptr };
			std::unordered_map<std::string, std::unique_ptr<rhi::Shader>> _shaderByRelativePath;
			std::unique_ptr<IShaderCompiler> _shaderCompiler{ nullptr };
			ShaderCache _shaderCache;
	};
}