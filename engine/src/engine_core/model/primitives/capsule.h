#pragma once

#include "core/math_base.h"

namespace ad_astris::ecore
{
	class Plane;
	class Ray;
	class Sphere;
	class AABB;
	
	class Capsule
	{
		public:
			Capsule() = default;
			Capsule(const AABB& aabb, float height);
			Capsule(const Sphere& sphere, float height);
			Capsule(const XMFLOAT3& base, const XMFLOAT3& tip, float radius);

			void create(const AABB& aabb, float height);
			void create(const Sphere& sphere, float height);
			void create(const XMFLOAT3& base, const XMFLOAT3& tip, float radius);

			bool intersects(const Capsule& other, XMFLOAT3& outPosition, XMFLOAT3& outIncidentNormal, float& outPenetrationDepth) const;
			bool intersects(const Sphere& sphere) const;
			bool intersects(const Sphere& sphere, float& outDistance) const;
			bool intersects(const Sphere& sphere, float& outDistance, XMFLOAT3& outDirection) const;
			bool intersects(const Plane& plane) const;
			bool intersects(const Plane& plane, float& outDistance) const;
			bool intersects(const Plane& plane, float& outDistance, XMFLOAT3& outDirection) const;
			bool intersects(const Ray& ray) const;
			bool intersects(const Ray& ray, float& outDistance) const;
			bool intersects(const Ray& ray, float& outDistance, XMFLOAT3& outDirection) const;

			AABB get_aabb() const;

			XMFLOAT3 get_base() const { return _base; }
			XMFLOAT3 get_tip() const { return _tip; }
			float get_radius() const { return _radius; }

		private:
			XMFLOAT3 _base{ 0, 0, 0};
			XMFLOAT3 _tip{ 0, 0, 0 };
			float _radius{ 0 };
	};
}