#include "material_template.h"
#include "shader.h"
#include "resource_manager/resource_visitor.h"

using namespace ad_astris;
using namespace ecore;

MaterialTemplate::MaterialTemplate(material::MaterialTemplateInfo& templateInfo, ObjectName* objectName)
	: _templateInfo(templateInfo)
{
	_name = objectName;
}

void MaterialTemplate::serialize(io::IFile* file)
{
	std::string newMetadata = material::Utils::pack_general_material_template_info(_templateInfo);
	file->set_metadata(newMetadata);
}

void MaterialTemplate::deserialize(io::IFile* file, ObjectName* templateName)
{
	_templateInfo = material::Utils::unpack_general_material_template_info(file->get_metadata());
	_name = templateName;
	_path = file->get_file_path();
}

inline void MaterialTemplate::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}
