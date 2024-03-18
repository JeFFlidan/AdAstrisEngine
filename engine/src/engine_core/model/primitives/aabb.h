#pragma once

#include "core/math_base.h"
#include "core/constants.h"
#include <vector>

namespace ad_astris::ecore
{
	class Ray;
	class Sphere;
	
	class AABB
	{
		public:
			enum class IntersectionType
			{
				INSIDE,
				INTERSECTS,
				OUTSIDE,
			};
		
			AABB() = default;
			AABB(const Sphere& sphere);
			AABB(const std::vector<XMFLOAT3>& vertexPositions);
			AABB(const XMFLOAT3& center, const XMFLOAT3& halfWidth);

			void create(const Sphere& sphere);
			void create(const std::vector<XMFLOAT3>& vertexPositions);
			void create(const XMFLOAT3& center, const XMFLOAT3& halfWidth);

			AABB transform(const XMMATRIX& mat) const;
			AABB transform(const XMFLOAT4X4& mat) const;

			XMMATRIX get_unorm_remap_matrix() const;
			XMFLOAT3 get_center() const;
			XMFLOAT3 get_half_width() const;
			float get_radius() const;

			IntersectionType intersects(const AABB& aabb) const;
			bool intersects(const XMFLOAT3& point) const;
			bool intersects(const Ray& ray) const;
			bool intersects(const Sphere& sphere) const;

			bool is_valid() const;
		
			XMFLOAT3 get_min() const { return _min; }
			XMFLOAT3 get_max() const { return _max; }

			static AABB merge(const AABB& a, const AABB& b);
		
		private:
			XMFLOAT3 _min{ FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
			XMFLOAT3 _max{ FLOAT_MIN, FLOAT_MIN, FLOAT_MIN };
	};
}