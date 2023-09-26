#pragma once

#include "material_base.h"
#include "material_common.h"
#include "resource_manager/resource_manager.h"

namespace ad_astris::ecore
{
	class OpaquePBRMaterial : public MaterialBase
	{
		public:
			OpaquePBRMaterial();
			// Constructor to create new OpaquePBRMaterial
			OpaquePBRMaterial(OpaquePBRMaterialSettings& materialSettings, ObjectName* materialName);

			OpaquePBRMaterial(const OpaquePBRMaterial& other) = delete;
			OpaquePBRMaterial& operator=(const OpaquePBRMaterial& other) = delete;
			
			OpaquePBRMaterialSettings* get_material_settings()
			{
				return static_cast<OpaquePBRMaterialSettings*>(_materialSettings.get());
			}

			virtual void accept(resource::IResourceVisitor& resourceVisitor) override;
	};

	class TransparentMaterial : public MaterialBase
	{
		public:
			TransparentMaterial();
			TransparentMaterial(TransparentMaterialSettings& materialSettings, ObjectName* materialName);

			TransparentMaterial(const TransparentMaterial& other) = delete;
			TransparentMaterial& operator=(const TransparentMaterial& other) = delete;

			TransparentMaterialSettings* get_material_settings()
			{
				return static_cast<TransparentMaterialSettings*>(_materialSettings.get());
			}

			virtual void accept(resource::IResourceVisitor& resourceVisitor) override;
	};
}

namespace ad_astris::resource
{
	template<>
	struct FirstCreationContext<ecore::OpaquePBRMaterial>
	{
		ecore::OpaquePBRMaterialSettings materialSettings;
		std::string materialName;
		io::URI materialPath;
	};

	template<>
	struct FirstCreationContext<ecore::TransparentMaterial>
	{
		ecore::TransparentMaterialSettings materialSettings;
		std::string materialName;
		io::URI materialPath;
	};
}
