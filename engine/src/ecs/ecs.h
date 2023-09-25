/* ====================================================
	This file must be included in the main program if
	you want to use the Entity-Component-System.
   ======================================================*/

#pragma once

#include "entity_manager.h"
#include "core/serialization.h"
#include "core/reflection.h"

#define REFLECT_COMPONENT(Type, ...)	\
	REFLECT_SERIALIZABLE_FIELDS(Type, __VA_ARGS__)
