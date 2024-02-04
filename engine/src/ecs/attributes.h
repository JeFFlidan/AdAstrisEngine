#pragma once

#include "core/compile_time_hash.h"
#include "core/reflection.h"
#include "core/common.h"

namespace ad_astris
{
#define GET_TYPE_ID(Type) compile_time_fnv1(#Type)

#define DEFINE_ATTR_BODY(Type)										\
	public:															\
		constexpr explicit Type(uint64_t id) : _id(id) { }			\
		FORCE_INLINE uint64_t get_id() const { return _id; }		\
	private:														\
		uint64_t _id;
	
	class EcsComponent
	{
		DEFINE_ATTR_BODY(EcsComponent)
	};

	class EcsTag
	{
		DEFINE_ATTR_BODY(EcsTag)
	};

	class EcsSystem
	{
		DEFINE_ATTR_BODY(EcsSystem)
	};

#define COMPONENT_REFLECTOR_START(Type)							\
	REFLECTOR_START(Type, EcsComponent(GET_TYPE_ID(Type)))

#define TAG_REFLECTOR(Type)										\
	REFLECTOR_START(Type, EcsTag(GET_TYPE_ID(Type)))			\
	REFLECTOR_END()
}