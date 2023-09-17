#pragma once

#include "engine/render_core_module.h"

namespace ad_astris::rcore::impl
{
	class ShaderCache
	{
		public:
			ShaderCache() = default;
			ShaderCache(ShaderCompilerInitContext& initContext);
		
			bool is_shader_outdated(const io::URI& shaderRelativePath, bool isEngineShader);
			void update_shader_cache(ShaderInputDesc& inputDesc, ShaderOutputDesc& outputDesc, bool isEngineShader);
			void load_shader_bin(const std::string& shaderName, std::vector<uint8_t>& outputData, bool isEngineShader);

			ShaderCacheType get_cache_type()
			{
				return _cacheType;
			}

		private:
			io::FileSystem* _fileSystem;
			ShaderCacheType _cacheType;

			void get_shader_object_relative_path(std::string& shaderName, io::URI& output);
	};
}