#pragma once
#include "rhi/resources.h"
#include "api.h"
#include <unordered_map>

namespace ad_astris::rcore::impl
{
	class RENDER_CORE_API ShaderCache
	{
		public:
			~ShaderCache();
			bool check_in_cache(rhi::ShaderInfo* shaderInfo, void* data, uint64_t size);
			void add_to_cache(rhi::ShaderInfo* shaderInfo, void* dataFromFile, uint64_t dataSize);
			
		private:
			struct DataFromFile
			{
				DataFromFile(uint8_t* data, uint64_t size);
				uint8_t bytes[8];
				uint64_t size;

				bool operator==(const DataFromFile& other) const;
				size_t hash() const;
			};

			struct DataFromFileHash
			{
				std::size_t operator()(const DataFromFile& k) const
				{
					return k.hash();
				}
			};

			std::unordered_map<DataFromFile, rhi::ShaderInfo, DataFromFileHash> _cache;
	};
}
