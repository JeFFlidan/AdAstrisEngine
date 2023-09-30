#pragma once

#include "events/event.h"
#include "engine_core/object.h"
#include "core/math_base.h"

namespace ad_astris::acore
{
	class ViewportResizedEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ViewportResizedEvent)

			ViewportResizedEvent(float width, float height) : _width(width), _height(height) { }
			float get_width() { return _width; }
			float get_height() { return _height; }
			bool is_extent_valid() { return _width && _height; }

		private:
			float _width{ 0.0f };
			float _height{ 0.0f };
	};

	struct ViewportState
	{
		bool isHovered{ false };
		XMFLOAT2 viewportMin{ 0, 0 };
		XMFLOAT2 viewportMax{ 0, 0 };
	};

	class ViewportHoverEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ViewportHoveredEvent)
			ViewportHoverEvent(ViewportState& viewportState) : _state(viewportState) { }
			ViewportState get_viewport_state() { return _state; }

		private:
			ViewportState _state;
	};

	class ResourceImportEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ResourceImportStartEvent)
			ResourceImportEvent(io::URI resourcePath, io::URI aaresPath) : _resourcePath(resourcePath), _aaresPath(aaresPath) { }

			io::URI get_resource_path() { return _resourcePath; }
			io::URI get_aares_path() { return _aaresPath; }

		private:
			io::URI _resourcePath;
			io::URI _aaresPath;
	};
	
	class ProjectSavingStartEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ProjectSaveEvent)
	};

	class ProjectSavingFinishEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(ProjectStartSavingEvent);
	};

	class OpaquePBRMaterialCreationEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(OpaquePBRMaterialCreationEvent);
			OpaquePBRMaterialCreationEvent(
				io::URI materialPath,
				const std::string& materialName,
				UUID albedoUUID,
				UUID normalUUID,
				UUID roughnessUUID,
				UUID metallicUUID,
				UUID aoUUID) : _materialPath(materialPath), _materialName(materialName), _albedo(albedoUUID), _normal(normalUUID), _roughness(roughnessUUID), _metallic(metallicUUID), _ao(aoUUID) { }

			UUID get_albedo_texture_uuid() { return _albedo; }
			UUID get_normal_texture_uuid() { return _normal; }
			UUID get_roughness_texture_uuid() { return _roughness; }
			UUID get_metallic_texture_uuid() { return _metallic; }
			UUID get_ao_texture_uuid() { return _ao; }
			std::string get_material_name() { return _materialName; }
			io::URI get_material_path() { return _materialPath; } 

		private:
			std::string _materialName;
			io::URI _materialPath;
			UUID _albedo, _normal, _metallic, _roughness, _ao;
	};

	class EditorPointLightCreationEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(EditorPointLightCreationEvent)
	};

	class EditorStaticModelCreationEvent : public events::IEvent
	{
		public:
			EVENT_TYPE_DECL(EditorStaticModelCreationEvent)
			EditorStaticModelCreationEvent(UUID modelUUID, UUID materialUUID) : _modelUUID(modelUUID), _materialUUID(materialUUID) { }

			UUID get_model_uuid() { return _modelUUID; }
			UUID get_material_uuid() { return _materialUUID; }

		private:
			UUID _modelUUID;
			UUID _materialUUID;
	};
}