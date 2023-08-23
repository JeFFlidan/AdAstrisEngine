#include "materials.h"

#include "resource_manager/resource_visitor.h"

using namespace ad_astris::ecore;

OpaquePBRMaterial::OpaquePBRMaterial()
{
	_materialSettings = std::make_unique<OpaquePBRMaterialSettings>();
}

OpaquePBRMaterial::OpaquePBRMaterial(OpaquePBRMaterialSettings& materialSettings, ObjectName* materialName, UUID generalMaterialTemplate)
	: MaterialBase(materialName, generalMaterialTemplate)
{
	_materialSettings = std::make_unique<OpaquePBRMaterialSettings>(materialSettings);
}

void OpaquePBRMaterial::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}

TransparentMaterial::TransparentMaterial()
{
	_materialSettings = std::make_unique<TransparentMaterialSettings>();
}

TransparentMaterial::TransparentMaterial(TransparentMaterialSettings& materialSettings, ObjectName* materialName, UUID generalMaterialTemplate)
	: MaterialBase(materialName, generalMaterialTemplate)
{
	_materialSettings = std::make_unique<TransparentMaterialSettings>(materialSettings);
}

void TransparentMaterial::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}
