#include "engine_core/model/static_model.h"
#include "engine_core/texture/texture2D.h"
#include "engine_core/material/materials.h"
#include "engine_core/material/shader.h"
#include "engine_core/level/level.h"
#include "file_system/file.h"

using namespace ad_astris;
using namespace resource;

void ResourceDeleterVisitor::visit(ecore::StaticModel* staticModel)
{
	_resourcePool->free(staticModel);
}
		
void ResourceDeleterVisitor::visit(ecore::Texture2D* texture2D)
{
	_resourcePool->free(texture2D);		
}
		
void ResourceDeleterVisitor::visit(ecore::Shader* shader)
{
	_resourcePool->free(shader);
}
		
void ResourceDeleterVisitor::visit(ecore::Level* level)
{
	_resourcePool->free(level);
}

void ResourceDeleterVisitor::visit(ecore::OpaquePBRMaterial* material)
{
	_resourcePool->free(material);
}

void ResourceDeleterVisitor::visit(ecore::TransparentMaterial* material)
{
	_resourcePool->free(material);
}

void ResourceDeleterVisitor::visit(resource::ResourceFile* resourceFile)
{
	_resourcePool->free(resourceFile);
}

void ResourceDeleterVisitor::visit(resource::LevelFile* levelFile)
{
	_resourcePool->free(levelFile);
}
