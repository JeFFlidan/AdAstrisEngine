#include "aabb.h"
#include "ray.h"
#include "sphere.h"

using namespace ad_astris;
using namespace ecore;

AABB::AABB(const Sphere& sphere)
{
	create(sphere);
}

AABB::AABB(const std::vector<XMFLOAT3>& vertexPositions)
{
	create(vertexPositions);
}

AABB::AABB(const XMFLOAT3& center, const XMFLOAT3& halfWidth)
{
	create(center, halfWidth);
}

void AABB::create(const std::vector<XMFLOAT3>& vertexPositions)
{
	_min = { FLOAT_MAX, FLOAT_MAX, FLOAT_MAX };
	_max = { FLOAT_MIN, FLOAT_MIN, FLOAT_MIN };

	for (auto& pos : vertexPositions)
	{
		_min = math::min(_min, pos);
		_max = math::max(_max, pos);
	}
}

void AABB::create(const XMFLOAT3& center, const XMFLOAT3& halfWidth)
{
	_min = XMFLOAT3(center.x - halfWidth.x, center.y - halfWidth.y, center.z - halfWidth.z);
	_max = XMFLOAT3(center.x + halfWidth.x, center.y + halfWidth.y, center.z + halfWidth.z);
}

void AABB::create(const Sphere& sphere)
{
	create(sphere.get_center(), XMFLOAT3(sphere.get_radius(), sphere.get_radius(), sphere.get_radius()));
}

AABB AABB::transform(const XMMATRIX& mat) const
{
	const XMVECTOR corners[8] = {
		XMVector3Transform(XMLoadFloat3(&_min), mat),
		XMVector3Transform(XMVectorSet(_min.x, _max.y, _min.z, 1), mat),
		XMVector3Transform(XMVectorSet(_min.x, _max.y, _max.z, 1), mat),
		XMVector3Transform(XMVectorSet(_min.x, _min.z, _max.z, 1), mat),
		XMVector3Transform(XMVectorSet(_max.x, _min.y, _max.z, 1), mat),
		XMVector3Transform(XMVectorSet(_max.x, _min.y, _min.z, 1), mat),
		XMVector3Transform(XMVectorSet(_max.x, _max.y, _min.z, 1), mat),
		XMVector3Transform(XMLoadFloat3(&_max), mat)
	};

	XMVECTOR min = corners[0];
	XMVECTOR max = corners[0];
	for (size_t i = 1; i != 8; ++i)
	{
		min = XMVectorMin(min, corners[i]);
		max = XMVectorMax(max, corners[i]);
	}

	AABB outputAABB;
	XMStoreFloat3(&outputAABB._min, min);
	XMStoreFloat3(&outputAABB._max, max);
	return outputAABB;
}

AABB AABB::transform(const XMFLOAT4X4& mat) const
{
	return transform(XMLoadFloat4x4(&mat));
}

XMMATRIX AABB::get_unorm_remap_matrix() const
{
	return XMMatrixMultiply(XMMatrixScaling(_max.x - _min.x, _max.y - _min.y, _max.z - _min.z),
		XMMatrixTranslation(_min.x, _min.y, _min.z));
}

XMFLOAT3 AABB::get_center() const
{
	return XMFLOAT3((_min.x + _max.x) * 0.5f, (_min.y + _max.y) * 0.5f, (_min.z + _max.z) * 0.5f);
}

XMFLOAT3 AABB::get_half_width() const
{
	XMFLOAT3 center = get_center();
	return XMFLOAT3(abs(_max.x - center.x), abs(_max.y - center.y), abs(_max.z - center.z));
}

float AABB::get_radius() const
{
	XMFLOAT3 halfWidth = get_half_width();
	return std::sqrt(std::pow(halfWidth.x, 2.0f) + std::pow(halfWidth.y, 2.0f) + std::pow(halfWidth.z, 2.0f));
}

AABB::IntersectionType AABB::intersects(const AABB& aabb) const
{
	if (!is_valid() && !aabb.is_valid())
		return IntersectionType::OUTSIDE;
	
	const XMFLOAT3& aMin = _min, &bMin = aabb._min;
	const XMFLOAT3& aMax = _max, &bMax = aabb._max;

	if (aMin.x >= bMin.x && aMin.y >= bMin.y && aMin.z >= bMin.z
		&& aMax.x <= bMax.x && aMax.y <= bMax.y && aMax.z <= bMax.z)
	{
		return IntersectionType::INSIDE;
	}

	if (aMax.x < bMin.x || aMin.x > bMax.x
		|| aMax.y < bMin.y || aMin.y > bMax.y
		|| aMax.z < bMax.z || aMin.z > aMax.z)
	{
		return IntersectionType::OUTSIDE;
	}

	return IntersectionType::INTERSECTS;
}

bool AABB::intersects(const XMFLOAT3& point) const
{
	if (!is_valid())
		return false;
	if (point.x > _max.x) return false;
	if (point.x < _min.x) return false;
	if (point.y > _max.y) return false;
	if (point.y < _min.y) return false;
	if (point.z > _max.z) return false;
	if (point.z < _min.z) return false;
	return true;
}

bool AABB::intersects(const Ray& ray) const
{
	if (!is_valid())
		return false;

	return true;
}

bool AABB::intersects(const Sphere& sphere) const
{
	return sphere.intersects(*this);
}

bool AABB::is_valid() const
{
	if (_min.x > _max.x || _min.y > _max.y || _min.z > _max.z)
		return false;
	return true;
}

AABB AABB::merge(const AABB& a, const AABB& b)
{
	AABB aabb;
	aabb._min = math::min(a._min, b._min);
	aabb._max = math::max(a._max, b._max);
	return aabb;
}
