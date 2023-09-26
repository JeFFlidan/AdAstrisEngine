#pragma once

namespace ad_astris
{
	namespace resource
	{
		template<typename T>
		class ResourceAccessor;

		class IResourceVisitor;
	}

	namespace ecore
	{
		class StaticModel;
		class Texture2D;
		class Shader;
		class Level;
		class MaterialTemplate;
		class OpaquePBRMaterial;
		class TransparentMaterial;

		typedef resource::ResourceAccessor<StaticModel> StaticModelHandle;
		typedef resource::ResourceAccessor<Texture2D> Texture2DHandle;
		typedef resource::ResourceAccessor<Shader> ShaderHandle;
		typedef resource::ResourceAccessor<Level> LevelHandle;
		typedef resource::ResourceAccessor<OpaquePBRMaterial> OpaquePBRMaterialHandle;
		typedef resource::ResourceAccessor<TransparentMaterial> TransparentMaterialHandle;
	}
}