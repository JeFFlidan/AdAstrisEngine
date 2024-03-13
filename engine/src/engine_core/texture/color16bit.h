#pragma once

#include "core/math_base.h"

namespace ad_astris::ecore
{
	struct Color16Bit
	{
		uint64_t rgba = 0;
		
		Color16Bit(uint64_t rgba) : rgba(rgba) { }
		Color16Bit(uint16_t r = 0, uint16_t g = 0, uint16_t b = 0, uint16_t a = 65535)
			: rgba((uint64_t)r | (uint64_t)g << 16 | (uint64_t)b << 32 | (uint64_t)a << 48) { }
		
		Color16Bit(const XMFLOAT3& color) : Color16Bit(
			uint16_t(color.x * 65535.0f),
			uint16_t(color.y * 65535.0f),
			uint16_t(color.z * 65535.0f)) { }
		
		Color16Bit(const XMFLOAT4& color) : Color16Bit(
			uint16_t(color.x * 65535.0f),
			uint16_t(color.y * 65535.0f),
			uint16_t(color.z * 65535.0f),
			uint16_t(color.w * 65535.0f)) { }

		uint16_t get_r() const { return rgba >> 0 & 0xFFFF; }
		uint16_t get_g() const { return rgba >> 16 & 0xFFFF; }
		uint16_t get_b() const { return rgba >> 32 & 0xFFFF; }
		uint16_t get_a() const { return rgba >> 48 & 0xFFFF; }

		float get_nr() const { return static_cast<float>(get_r()) / 65535.0f; }
		float get_ng() const { return static_cast<float>(get_g()) / 65535.0f; }
		float get_nb() const { return static_cast<float>(get_b()) / 65535.0f; }
		float get_na() const { return static_cast<float>(get_a()) / 65535.0f; }

		void set_r(uint16_t r) { *this = { r, get_g(), get_b(), get_a() }; }
		void set_g(uint16_t g) { *this = { get_r(), g, get_b(), get_a() }; }
		void set_b(uint16_t b) { *this = { get_r(), get_g(), b, get_a() }; }
		void set_a(uint16_t a) { *this = { get_r(), get_g(), get_b(), a }; }

		XMFLOAT3 to_float3() const
		{
			return XMFLOAT3{ get_nr(), get_ng(), get_nb() };
		}

		XMFLOAT4 to_float4() const
		{
			return XMFLOAT4{ get_nr(), get_ng(), get_nb(), get_na() };
		}

		operator XMFLOAT3() const { return to_float3(); }
		operator XMFLOAT4() const { return to_float4(); }
		operator uint64_t() const { return rgba; }
	};
}