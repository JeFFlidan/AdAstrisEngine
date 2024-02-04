/* ====================================================
	This file must be included in the main program if
	you want to use the Entity-Component-System.
   ======================================================*/

#pragma once

#include "entity_manager.h"
#include "core/serialization.h"
#include "core/reflection.h"
#include "ui_core/ecs_ui_manager.h"
#include "attributes.h"

#define REFLECT_COMPONENT(Type, ...)	\
	REFLECT_SERIALIZABLE_FIELDS(Type, __VA_ARGS__)

namespace ad_astris::ecs
{
	template<typename T>
	void register_component(EntityManager* entityManager, uicore::ECSUiManager* ecsUiManager)
	{
		entityManager->register_component<T>(true);
		ecsUiManager->register_component<T>();
	}
}
