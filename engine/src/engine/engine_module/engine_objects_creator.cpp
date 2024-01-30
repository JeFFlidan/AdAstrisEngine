#include "engine_objects_creator.h"
#include "engine_core/default_object_creators.h"
#include "engine_core/basic_components.h"
#include "core/global_objects.h"
#include "application_core/editor_events.h"

using namespace ad_astris;
using namespace engine::impl;

EngineObjectsCreator::EngineObjectsCreator()
{
	subscribe_to_events();
}

void EngineObjectsCreator::create_new_objects()
{
	for (auto& context : _staticPointLightsToCreate)
		ecore::StaticPointLightCreator::create(context);

	for (auto& context : _staticModelsToCreate)
		ecore::StaticModelCreator::create(context);

	_staticPointLightsToCreate.clear();
	_staticModelsToCreate.clear();
}

void EngineObjectsCreator::subscribe_to_events()
{
	events::EventManager* eventManager = EVENT_MANAGER();
	
	events::EventDelegate<acore::EditorPointLightCreationEvent> delegate1 = [&](acore::EditorPointLightCreationEvent& event)
	{
		ecore::EditorObjectCreationContext& objectCreationContext = _staticPointLightsToCreate.emplace_back();
		objectCreationContext.location = XMFLOAT3(0.0f, 0.0f, 0.0f);
	};
	eventManager->subscribe(delegate1);

	events::EventDelegate<acore::EditorStaticModelCreationEvent> delegate2 = [&](acore::EditorStaticModelCreationEvent& event)
	{
		ecore::EditorObjectCreationContext& objectCreationContext = _staticModelsToCreate.emplace_back();
		objectCreationContext.materialUUID = event.get_material_uuid();
		objectCreationContext.uuid = event.get_model_uuid();
	};
	eventManager->subscribe(delegate2);
}
