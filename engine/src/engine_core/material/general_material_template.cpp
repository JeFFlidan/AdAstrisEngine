#include "general_material_template.h"
#include "general_material_template.h"
#include "rhi/resources.h"
#include "shader.h"

using namespace ad_astris;
using namespace ecore;

GeneralMaterialTemplate::GeneralMaterialTemplate(material::GeneralMaterialTemplateInfo& templateInfo, ObjectName* objectName)
	: _templateInfo(templateInfo)
{
	_name = objectName;
}

void GeneralMaterialTemplate::serialize(io::IFile* file)
{
	std::string newMetadata = material::Utils::pack_general_material_template_info(_templateInfo);
	file->set_metadata(newMetadata);
}

void GeneralMaterialTemplate::deserialize(io::IFile* file, ObjectName* templateName)
{
	_templateInfo = material::Utils::unpack_general_material_template_info(file->get_metadata());
	_name = templateName;
	_path = file->get_file_path();
}
