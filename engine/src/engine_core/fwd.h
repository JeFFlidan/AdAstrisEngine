#pragma once

namespace ad_astris
{
	namespace resource
	{
		template<typename T>
		class ResourceAccessor;
	}

	namespace ecore
	{
		class StaticModel;
		class Texture2D;
		class Shader;
		class Level;
		class GeneralMaterialTemplate;

		typedef resource::ResourceAccessor<StaticModel> StaticModelHandle;
		typedef resource::ResourceAccessor<Texture2D> Texture2DHandle;
		typedef resource::ResourceAccessor<Shader> ShaderHandle;
		typedef resource::ResourceAccessor<GeneralMaterialTemplate> GeneralMaterialTemplateHandle;
		typedef resource::ResourceAccessor<Level> LevelHandle;
	}
}