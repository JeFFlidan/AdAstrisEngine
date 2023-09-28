#pragma once

#include "core/serialization.h"
#include "core/reflection.h"
#include "engine_core/uuid.h"

namespace ad_astris::ecore
{
#define REGISTER_MATERIAL_SETTINGS(Type, ...) REFLECT_SERIALIZABLE_FIELDS(Type, __VA_ARGS__)

#define IMPLEMENT_MATERIAL_SETTINGS_METHODS()										\
	virtual std::string serialize() override										\
	{																				\
		return serialization::serialize_to_json(*this);								\
	}																				\
	virtual void deserialize(const std::string& materialSettingsJsonStr) override	\
	{																				\
		serialization::deserialize_from_json(materialSettingsJsonStr, *this);		\
	}																				\
	virtual uint32_t get_size() override											\
	{																				\
		return sizeof(*this);														\
	}

	struct IMaterialSettings
	{
		virtual std::string serialize() = 0;
		virtual void deserialize(const std::string& materialSettingsJsonStr) = 0;
		virtual uint32_t get_size() = 0;
	};
	
	struct OpaquePBRMaterialSettings : IMaterialSettings
	{
		IMPLEMENT_MATERIAL_SETTINGS_METHODS()
		UUID baseColorTextureUUID;
		UUID roughnessTextureUUID;
		UUID metallicTextureUUID;
		UUID ambientOcclusionTextureUUID;
		UUID normalTextureUUID;
		XMFLOAT4 tintColor;
		float roughnessValue;
		float metallicValue;
		bool useTintColor{ false };
		bool useRoughnessValue{ false };
		bool useMetallicValue{ false };
	};

	struct TransparentMaterialSettings : IMaterialSettings
	{
		IMPLEMENT_MATERIAL_SETTINGS_METHODS()
		UUID baseColorTextureUUID;
		UUID opacityTextureUUID;
		UUID ambientOcclusionTextureUUID;
		XMFLOAT4 tintColor;
		float opacityValue;
		bool useTintColor{ false };
		bool useOpacityValue{ false };
	};
}

REGISTER_MATERIAL_SETTINGS(
	ad_astris::ecore::OpaquePBRMaterialSettings,
	baseColorTextureUUID,
	roughnessTextureUUID,
	metallicTextureUUID,
	ambientOcclusionTextureUUID,
	normalTextureUUID,
	tintColor, 
	roughnessValue,
	metallicValue,
	useTintColor,
	useRoughnessValue,
	useMetallicValue)
REGISTER_MATERIAL_SETTINGS(ad_astris::ecore::TransparentMaterialSettings, tintColor)
