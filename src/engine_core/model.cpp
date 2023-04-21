#include "model.h"
#include "resource_manager/utils.h"

using namespace ad_astris;

void* resource::ResourceMethods<ecore::Model>::create(void* data)
{
	ResourceInfo* resourceInfo = static_cast<ResourceInfo*>(data);
	ModelInfo* modelInfo = utils::unpack_model_info(resourceInfo);
	
	ecore::Model* model = new ecore::Model();
	model->info = modelInfo;
	model->vertices = resourceInfo->data;
	model->verticesCount = modelInfo->vertexBufferSize;
	model->indices = resourceInfo->data + modelInfo->vertexBufferSize;
	model->indicesCount = modelInfo->indexBufferSize;
	return model;
}

void resource::ResourceMethods<ecore::Model>::destroy(void* resource)
{
	ecore::Model* model = static_cast<ecore::Model*>(resource);
	delete[] model->vertices;
	delete model->info;
	delete model;
}
