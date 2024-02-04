#pragma once

#include "core/serialization.h"
#include "core/reflection.h"
#include "engine_core/uuid.h"

namespace ad_astris::ecore
{
#define REGISTER_MATERIAL_SETTINGS(Type, ...) REFLECT_SERIALIZABLE_FIELDS(Type, __VA_ARGS__)

#define IMPLEMENT_MATERIAL_SETTINGS_METHODS()										\
	virtual nlohmann::json serialize() override										\
	{																				\
		return serialization::serialize_to_json(*this);								\
	}																				\
	virtual void deserialize(const nlohmann::json& materialSettingsJsonStr) override\
	{																				\
		serialization::deserialize_from_json(materialSettingsJsonStr, *this);		\
	}																				\
	virtual uint32_t get_size() override											\
	{																				\
		return sizeof(*this);														\
	}

	struct IMaterialSettings
	{
		virtual nlohmann::json serialize() = 0;
		virtual void deserialize(const nlohmann::json& materialSettingsJsonStr) = 0;
		virtual uint32_t get_size() = 0;
	};
	
	struct OpaquePBRMaterialSettings : IMaterialSettings
	{
		IMPLEMENT_MATERIAL_SETTINGS_METHODS()
		REFLECTOR_START(OpaquePBRMaterialSettings)
		FIELD(UUID, baseColorTextureUUID, (), Serializable(), EditAnywhere())
		FIELD(UUID, roughnessTextureUUID, (), Serializable(), EditAnywhere())
		FIELD(UUID, metallicTextureUUID, (), Serializable(), EditAnywhere())
		FIELD(UUID, ambientOcclusionTextureUUID, (), Serializable(), EditAnywhere())
		FIELD(UUID, normalTextureUUID, (), Serializable(), EditAnywhere())
		FIELD(XMFLOAT4, tintColor, (), Serializable(), EditAnywhere())
		FIELD(float, roughnessValue, (), Serializable(), EditAnywhere())
		FIELD(float, metallicValue, (), Serializable(), EditAnywhere())
		FIELD(bool, useTintColor, (false), Serializable(), EditAnywhere())
		FIELD(bool, useRoughnessValue, (false), Serializable(), EditAnywhere())
		FIELD(bool, useMetallicValue, (false), Serializable(), EditAnywhere())
		REFLECTOR_END()
	};

	struct TransparentMaterialSettings : IMaterialSettings
	{
		IMPLEMENT_MATERIAL_SETTINGS_METHODS()
		REFLECTOR_START(TransparentMaterialSettings)
		UUID baseColorTextureUUID;
		UUID opacityTextureUUID;
		UUID ambientOcclusionTextureUUID;
		FIELD(XMFLOAT4, tintColor, (), Serializable(), EditAnywhere())
		float opacityValue;
		bool useTintColor{ false };
		bool useOpacityValue{ false };
		REFLECTOR_END()
	};
}
