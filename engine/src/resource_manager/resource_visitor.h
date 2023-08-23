#pragma once

#include "resource_pool.h"

namespace ad_astris
{
	namespace ecore
	{
		class StaticModel;
		class Texture2D;
		class MaterialTemplate;
		class Shader;
		class Level;
		class OpaquePBRMaterial;
		class TransparentMaterial;
	}

	namespace io
	{
		class ResourceFile;
		class LevelFile;
	}
}

namespace ad_astris::resource
{
	class IResourceVisitor
	{
		public:
			virtual ~IResourceVisitor() { }
			virtual void visit(ecore::StaticModel* staticModel) = 0;
			virtual void visit(ecore::Texture2D* texture2D) = 0;
			virtual void visit(ecore::MaterialTemplate* materialTemplate) = 0;
			virtual void visit(ecore::Shader* shader) = 0;
			virtual void visit(ecore::Level* level) = 0;
			virtual void visit(ecore::OpaquePBRMaterial* material) = 0;
			virtual void visit(ecore::TransparentMaterial* material) = 0;
			virtual void visit(io::ResourceFile* resourceFile) = 0;
			virtual void visit(io::LevelFile* levelFile) = 0;
	};

	class ResourceDeleterVisitor : public IResourceVisitor
	{
		public:
			ResourceDeleterVisitor() = default;
			ResourceDeleterVisitor(ResourcePool* resourcePool) : _resourcePool(resourcePool) { }

			virtual void visit(ecore::StaticModel* staticModel) override;
			virtual void visit(ecore::Texture2D* texture2D) override;
			virtual void visit(ecore::MaterialTemplate* materialTemplate) override;
			virtual void visit(ecore::Shader* shader) override;
			virtual void visit(ecore::Level* level) override;
			virtual void visit(ecore::OpaquePBRMaterial* material) override;
			virtual void visit(ecore::TransparentMaterial* material) override;
			virtual void visit(io::ResourceFile* resourceFile) override;
			virtual void visit(io::LevelFile* levelFile) override;

		private:
			ResourcePool* _resourcePool;
	};
}
