#ifndef SHADER_INTEROP
#define SHADER_INTEROP

#ifdef __cplusplus

#include "core/math_base.h"

using float2 = XMFLOAT2;
using float3 = XMFLOAT3;
using float4 = XMFLOAT4;
using uint = uint32_t;
using uint2 = XMUINT2;
using uint3 = XMUINT3;
using uint4 = XMUINT4;
using int2 = XMINT2;
using int3 = XMINT3;
using int4 = XMINT4;
using float4x4 = XMFLOAT4X4;

#define UNIFORM_BUFFER(Name, Type, Slot)
#define PUSH_CONSTANT(Name, Type)

#else

#define PASTE1(a, b) a##b
#define PASTE(a, b) PASTE1(a, b)
#define UNIFORM_BUFFER(Name, Type, Slot) ConstantBuffer<Type> Name : register(PASTE(b, Slot))

#if defined(__spirv__)
#define PUSH_CONSTANT(Name, Type) [[vk::push_constant]] Type Name;
#endif

#endif // __cplusplus

#define UB_FRAME_SLOT		0
#define UB_CAMERA_SLOT		1

#endif // SHADER_INTEROP
