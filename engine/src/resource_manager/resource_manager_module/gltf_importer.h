#pragma once

#include "engine_core/model/model.h"

namespace ad_astris::resource::impl
{
	class GLTFImporter
	{
		public:
			static bool import(const std::string& path, ecore::ModelInfo& outModelInfo);
	};
}
