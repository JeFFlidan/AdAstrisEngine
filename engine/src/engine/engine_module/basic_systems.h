#pragma once

#include "ecs/system_manager.h"

namespace ad_astris::engine::impl
{
	class BasicSystemsUtils
	{
		public:
			static void translate(const XMFLOAT3& input, XMFLOAT3& output);
			static void rotate(const XMFLOAT3& input, XMFLOAT4& output);
			static void scale(const XMFLOAT3& input, XMFLOAT3& output);
	};
	
	class TransformUpdateSystem : public ecs::System
	{
		public:
			OVERRIDE_SYSTEM_METHODS()
	};

	class CameraUpdateSystem : public ecs::System
	{
		public:
			OVERRIDE_SYSTEM_METHODS()

		private:
			float _deltaTime;
			ecs::Entity _activeCamera;
			float _width = 0.0f;
			float _height = 0.0f;
			bool _isViewportHovered{ false };
	};
	
	void register_basic_systems(ecs::SystemManager* systemManager);
}
