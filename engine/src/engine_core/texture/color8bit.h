#pragma once

#include <core/math_base.h>

namespace ad_astris::ecore
{
	struct Color8Bit
	{
		uint32_t rgba = 0;
		
		Color8Bit(uint32_t rgba) : rgba(rgba) { }
		Color8Bit(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 255)
			: rgba((uint32_t)r | (uint32_t)g << 8 | uint32_t(b) << 16 | uint32_t(a) << 24) { }
		
		Color8Bit(const XMFLOAT3& color) : Color8Bit(
			uint8_t(color.x * 255.0f),
			uint8_t(color.y * 255.0f),
			uint8_t(color.z * 255.0f)) { }
		
		Color8Bit(const XMFLOAT4& color) : Color8Bit(
			uint8_t(color.x * 255.0f),
			uint8_t(color.y * 255.0f),
			uint8_t(color.z * 255.0f),
			uint8_t(color.w * 255.0f)) { }

		uint8_t get_r() const { return rgba >> 0 & 0xFF; }
		uint8_t get_g() const { return rgba >> 8 & 0xFF; }
		uint8_t get_b() const { return rgba >> 16 & 0xFF; }
		uint8_t get_a() const { return rgba >> 24 & 0xFF; }

		// Returns normalized data from red channel
		float get_nr() const { return static_cast<float>(get_r()) / 255.0f; }
		// Returns normalized data from green channel
		float get_ng() const { return static_cast<float>(get_g()) / 255.0f; }
		// Returns normalized data from blue channel
		float get_nb() const { return static_cast<float>(get_b()) / 255.0f; }
		// Returns normalized data from alpha channel
		float get_na() const { return static_cast<float>(get_a()) / 255.0f; }

		void set_r(uint8_t r) { *this = { r, get_r(), get_b(), get_a() }; }
		void set_g(uint8_t g) { *this = { get_r(), g, get_b(), get_a() }; }
		void set_b(uint8_t b) { *this = { get_r(), get_g(), b, get_a()}; }
		void set_a(uint8_t a) { *this = { get_r(), get_g(), get_b(), a }; }

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
		operator uint32_t() const { return rgba; }
	};
}