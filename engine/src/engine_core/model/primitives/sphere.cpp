#include "sphere.h"
#include "plane.h"
#include "aabb.h"
#include "capsule.h"
#include "ray.h"

using namespace ad_astris;
using namespace ecore;

Sphere::Sphere(const std::vector<XMFLOAT3>& vertexPositions)
{
	create(vertexPositions);
}

Sphere::Sphere(const AABB& aabb)
{
	create(aabb);
}

void Sphere::create(const std::vector<XMFLOAT3>& vertexPositions)
{
	AABB aabb(vertexPositions);
	_center = aabb.get_center();
	_radius = aabb.get_radius();
}

void Sphere::create(const AABB& aabb)
{
	_center = aabb.get_center();
	_radius = aabb.get_radius();
}

bool Sphere::intersects(const AABB& aabb) const
{
	if (!aabb.is_valid())
		return false;

	XMFLOAT3 min = aabb.get_min();
	XMFLOAT3 max = aabb.get_max();
	XMFLOAT3 closestPointToSphereCenterInAABB = math::min(math::max(_center, min), max);
	float distanceSquared = math::distance_squared(closestPointToSphereCenterInAABB, _center);

	return distanceSquared < std::pow(_radius, 2.0f);
}

bool Sphere::intersects(const Sphere& sphere) const
{
	float distance{ 0 };
	intersects(sphere, distance);
	return true;
}

bool Sphere::intersects(const Sphere& sphere, float& outDistance) const
{
	outDistance = math::distance(_center, sphere._center);
	outDistance = outDistance - _radius - sphere._radius;
	return outDistance < 0;
}

bool Sphere::intersects(const Sphere& sphere, float& outDistance, XMFLOAT3& outDirection) const
{
	XMVECTOR aCenter = XMLoadFloat3(&_center);
	XMVECTOR bCenter = XMLoadFloat3(&sphere._center);
	XMVECTOR direction = XMVectorSubtract(aCenter, bCenter);
	XMVECTOR distance = XMVector3Length(direction);
	direction = XMVectorDivide(direction, distance);
	XMStoreFloat3(&outDirection, direction);
	outDistance = XMVectorGetX(distance);
	outDistance = outDistance - _radius - sphere._radius;
	return outDistance < 0;
}

bool Sphere::intersects(const Capsule& capsule) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Capsule& capsule, float& outDistance) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Capsule& capsule, float& outDistance, XMFLOAT3& outDirection) const
{
	// TODO
	return true;	
}

bool Sphere::intersects(const Plane& plane) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Plane& plane, float& outDistance) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Plane& plane, float& outDistance, XMFLOAT3& outDirection) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Ray& ray) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Ray& ray, float& outDistance) const
{
	// TODO
	return true;
}

bool Sphere::intersects(const Ray& ray, float& outDistance, XMFLOAT3& outDirection) const
{
	// TODO
	return true;
}
