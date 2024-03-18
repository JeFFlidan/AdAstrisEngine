#pragma once

#include "core/math_base.h"
#include <vector>

namespace ad_astris::ecore
{
	class AABB;
	class Capsule;
	class Ray;
	class Plane;
	
	class Sphere
	{
		public:
			Sphere() = default;
			Sphere(const std::vector<XMFLOAT3>& vertexPositions);
			Sphere(const AABB& aabb);

			void create(const std::vector<XMFLOAT3>& vertexPositions);
			void create(const AABB& aabb);

			bool intersects(const AABB& aabb) const;
			bool intersects(const Sphere& sphere) const;
			bool intersects(const Sphere& sphere, float& outDistance) const;
			bool intersects(const Sphere& sphere, float& outDistance, XMFLOAT3& outDirection) const;
			bool intersects(const Capsule& capsule) const;
			bool intersects(const Capsule& capsule, float& outDistance) const;
			bool intersects(const Capsule& capsule, float& outDistance, XMFLOAT3& outDirection) const;
			bool intersects(const Plane& plane) const;
			bool intersects(const Plane& plane, float& outDistance) const;
			bool intersects(const Plane& plane, float& outDistance, XMFLOAT3& outDirection) const;
			bool intersects(const Ray& ray) const;
			bool intersects(const Ray& ray, float& outDistance) const;
			bool intersects(const Ray& ray, float& outDistance, XMFLOAT3& outDirection) const;

			XMFLOAT3 get_center() const { return _center; }
			float get_radius() const { return _radius; }
		
		private:
			XMFLOAT3 _center{ 0, 0, 0 };
			float _radius{ 0 };
	};
}