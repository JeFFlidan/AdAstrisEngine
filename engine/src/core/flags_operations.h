#pragma once
#include <type_traits>

// Code from: https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html
// Source was found: https://github.com/turanszkij/WickedEngine

template<typename E>
struct EnableBitMaskOperator
{
	static const bool enable = false;
};

template<typename E>
typename std::enable_if<EnableBitMaskOperator<E>::enable, E>::type operator|(E lValue, E rValue)
{
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(static_cast<underlying>(lValue) | static_cast<underlying>(rValue));
}

template<typename E>
typename std::enable_if<EnableBitMaskOperator<E>::enable, E&>::type operator|=(E& lValue, E rValue)
{
	typedef typename std::underlying_type<E>::type underlying;
	lValue = static_cast<E>(static_cast<underlying>(lValue) | static_cast<underlying>(rValue));
	return lValue;
}

template<typename E>
typename std::enable_if<EnableBitMaskOperator<E>::enable, E>::type operator&(E lValue, E rValue)
{
	typedef typename std::underlying_type<E>::type underlying;
	return static_cast<E>(static_cast<underlying>(lValue) & static_cast<underlying>(rValue));
}

template<typename E>
typename std::enable_if<EnableBitMaskOperator<E>::enable, E&>::type operator&=(E& lValue, E rValue)
{
	typedef typename std::underlying_type<E>::type underlying;
	lValue = static_cast<E>(static_cast<underlying>(lValue) & static_cast<underlying>(rValue));
	return lValue;
}

template<typename E>
typename std::enable_if<EnableBitMaskOperator<E>::enable, E&>::type operator~(E rValue)
{
	typedef typename std::underlying_type<E>::type underlying;
	rValue = static_cast<E>(~static_cast<underlying>(rValue));
	return rValue;
}

template<typename E>
constexpr bool has_flag(E lValue, E rValue)
{
	return (lValue & rValue) == rValue;
}

#define ENABLE_BIT_MASK(Type)				\
	template<>								\
	struct EnableBitMaskOperator<Type>		\
	{										\
		static const bool enable = true;	\
	};
