#include "capsule.h"
#include "sphere.h"
#include "aabb.h"
#include "ray.h"
#include "plane.h"

using namespace ad_astris;
using namespace ecore;

Capsule::Capsule(const AABB& aabb, float height)
{
	create(aabb, height);
}

Capsule::Capsule(const Sphere& sphere, float height)
{
	create(sphere, height);
}


Capsule::Capsule(const XMFLOAT3& base, const XMFLOAT3& tip, float radius)
	: _base(base), _tip(tip), _radius(radius)
{
	
}

void Capsule::create(const AABB& aabb, float height)
{
	XMFLOAT3 center = aabb.get_center();
	_radius = aabb.get_radius();
	assert(_radius >= 0);
	_base = { center.x, center.y - _radius, center.z };
	_tip = { _base.x, _base.y + height, _base.z };
}

void Capsule::create(const Sphere& sphere, float height)
{
	XMFLOAT3 center = sphere.get_center();
	_radius = sphere.get_radius();
	assert(_radius >= 0);
	_base = { center.x, center.y - _radius, center.z };
	_tip = { _base.x, _base.y + height, _base.z };
}

void Capsule::create(const XMFLOAT3& base, const XMFLOAT3& tip, float radius)
{
	_base = base;
	_tip = tip;
	_radius = radius;
}

// https://wickedengine.net/2020/04/26/capsule-collision-detection/
bool Capsule::intersects(const Capsule& other, XMFLOAT3& outPosition, XMFLOAT3& outIncidentNormal, float& outPenetrationDepth) const
{
	if (get_aabb().intersects(other.get_aabb()) == AABB::IntersectionType::OUTSIDE)
		return false;

	XMVECTOR aBase = XMLoadFloat3(&_base);
	XMVECTOR aTip = XMLoadFloat3(&_tip);
	XMVECTOR aRadius = XMLoadFloat(&_radius);
	XMVECTOR aNormal = XMVector3Normalize(XMVectorSubtract(aBase, aTip));
	XMVECTOR aLineEndOffset = XMVectorScale(aNormal, _radius);
	XMVECTOR aA = XMVectorAdd(aBase, aLineEndOffset);
	XMVECTOR aB = XMVectorAdd(aTip, aLineEndOffset);

	XMVECTOR bBase = XMLoadFloat3(&other._base);
	XMVECTOR bTip = XMLoadFloat3(&other._tip);
	XMVECTOR bRadius = XMLoadFloat(&other._radius);
	XMVECTOR bNormal = XMVector3Normalize(XMVectorSubtract(bBase, bTip));
	XMVECTOR bLineEndOffset = XMVectorScale(bNormal, other._radius);
	XMVECTOR bA = XMVectorAdd(bBase, bLineEndOffset);
	XMVECTOR bB = XMVectorAdd(bTip, bLineEndOffset);

	XMVECTOR vec0 = XMVectorSubtract(bA, aA);
	XMVECTOR vec1 = XMVectorSubtract(bB, aA);
	XMVECTOR vec2 = XMVectorSubtract(bA, aB);
	XMVECTOR vec3 = XMVectorSubtract(bB, aB);

	float squaredDistance0 = XMVectorGetX(XMVector3LengthSq(vec0));
	float squaredDistance1 = XMVectorGetX(XMVector3LengthSq(vec1));
	float squaredDistance2 = XMVectorGetX(XMVector3LengthSq(vec2));
	float squaredDistance3 = XMVectorGetX(XMVector3LengthSq(vec3));

	XMVECTOR bestA;
	if (squaredDistance2 < squaredDistance0 || squaredDistance2 < squaredDistance1
		|| squaredDistance3 < squaredDistance0 || squaredDistance3 < squaredDistance1)
	{
		bestA = aB;
	}
	else
	{
		bestA = aA;
	}

	XMVECTOR bestB = math::closest_point_on_line_segment(bA, bB, bestA);
	bestA = math::closest_point_on_line_segment(aA, aB, bestB);

	XMVECTOR normal = XMVectorSubtract(bestA, bestB);
	XMVECTOR length = XMVector3Length(normal);
	normal = XMVectorDivide(normal, length);
	float distance = XMVectorGetX(length);

	XMStoreFloat3(&outPosition, XMVectorSubtract(bestA, XMVectorScale(normal, _radius)));
	XMStoreFloat3(&outIncidentNormal, normal);
	outPenetrationDepth = _radius + other._radius - distance;
	return outPenetrationDepth > 0;
}

bool Capsule::intersects(const Sphere& sphere) const
{
	return true;
}

bool Capsule::intersects(const Sphere& sphere, float& outDistance) const
{
	return true;
}

bool Capsule::intersects(const Sphere& sphere, float& outDistance, XMFLOAT3& outDirection) const
{
	return true;
}

bool Capsule::intersects(const Plane& plane) const
{
	return true;
}

bool Capsule::intersects(const Plane& plane, float& outDistance) const
{
	return true;
}

bool Capsule::intersects(const Plane& plane, float& outDistance, XMFLOAT3& outDirection) const
{
	return true;
}

bool Capsule::intersects(const Ray& ray) const
{
	return true;
}

bool Capsule::intersects(const Ray& ray, float& outDistance) const
{
	return true;
}

bool Capsule::intersects(const Ray& ray, float& outDistance, XMFLOAT3& outDirection) const
{
	return true;
}

AABB Capsule::get_aabb() const
{
	AABB baseAABB(_base, XMFLOAT3(_radius, _radius, _radius));
	AABB tipAABB(_tip, XMFLOAT3(_radius, _radius, _radius));
	AABB result = AABB::merge(baseAABB, tipAABB);
	assert(result.is_valid());
	return result;
}
