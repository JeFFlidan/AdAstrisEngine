#pragma once

#include "reflection.h"
#include <type_traits>
#include <tuple>

template <typename T, typename Tuple>
struct TupleHasType;

template <typename T, typename... Us>
struct TupleHasType<T, std::tuple<Us...>> : std::disjunction<std::is_base_of<T, Us>...> {};

class Reflector;

template<typename ...ARGS>
constexpr auto setup_attributes(ARGS&&... args)
{
	return std::make_tuple(std::forward<ARGS>(args)...);
}

#define SETUP_ATTRIBUTES(...)												\
	constexpr static auto attributes = setup_attributes(__VA_ARGS__);		\
	using AttributeTypes = std::remove_const_t<decltype(attributes)>;		\
																			\
	template<typename T>													\
	static constexpr bool has_attribute()									\
	{																		\
		return TupleHasType<T, AttributeTypes>::value;						\
	}																		\
																			\
	template<typename T>													\
	static constexpr T get_attribute()										\
	{																		\
		return std::get<T>(attributes);										\
	}

#define REFLECT_TYPE_BASE(Type, ...)										\
	friend Reflector;                                                   	\
	private:																\
	    using type = Type;                                                  \
	    static constexpr size_t _fieldIndexOffset = __COUNTER__ + 1;       	\
																			\
	    template<size_t, typename = void>                                 	\
	    struct Field {};                                                    \
																			\
		template<size_t index>												\
		static constexpr Field<index> get_field()							\
		{																	\
			return Field<index>{};											\
		}																	\
																			\
		static constexpr const char* get_name()								\
		{																	\
			return #Type;													\
		}

#define REFLECTOR_START(Type, ...)											\
	REFLECT_TYPE_BASE(Type, __VA_ARGS__)									\
	SETUP_ATTRIBUTES(__VA_ARGS__)

#define REFLECTOR_END()														\
	static constexpr size_t _fieldCount = __COUNTER__ - _fieldIndexOffset;

#define REFLECT_FIELD_BASE(Type, Name, ...)									\
	using fieldType = Type;													\
	static constexpr auto pointer{&type::Name};								\
																			\
	static constexpr const char* get_name()									\
	{																		\
		return #Name;														\
	}																		\
																			\
	template<typename T>													\
	Type operator()(const T& target)										\
	{																		\
		return target.*(pointer);											\
	}																		\
																			\
	template<typename T, typename V>										\
	void operator()(T& target, V&& value)									\
	{																		\
		target.*(pointer) = std::forward<V>(value);							\
	}

#define REFLECT_FIELD(Type, Name, ...)										\
	private:																\
		template<typename __UNUSED>											\
		struct Field<__COUNTER__ - _fieldIndexOffset, __UNUSED>				\
		{																	\
			REFLECT_FIELD_BASE(Type, Name, __VA_ARGS__)						\
			SETUP_ATTRIBUTES(__VA_ARGS__)									\
		};

#define FIELD(Type, Name, ...)												\
	public:																	\
		Type Name;															\
	REFLECT_FIELD(Type, Name, __VA_ARGS__)

#define FIELD_DV(Type, Name, Value, ...)									\
	public:																	\
		Type Name = Type##Value;											\
	REFLECT_FIELD(Type, Name, __VA_ARGS__)

#define PRIVATE_FIELD(Type, Name, ...)										\
	private:																\
		Type Name;															\
	REFLECT_FIELD(Type, Name, __VA_ARGS__)

#define PRIVATE_FIELD_DV(Type, Name, Value, ...)							\
	private:																\
		Type Name = Type##Value;											\
	REFLECT_FIELD(Type, Name, __VA_ARGS__)

class Reflector
{
	public:
		template<typename T, typename Func>
		static constexpr void for_each(Func&& func)
		{
			execute_in_order<T, 0>(std::forward<Func>(func));
		}

		template<typename T, typename F>
		static decltype(auto) get_value(T& target, F& field)
		{
			return target.*(field.pointer);
		}

		template<typename T, typename F, typename V>
		static void set_value(T& target, F& field, V&& value)
		{
			target.*(field.pointer) = std::forward<V>(value);
		}

		template<typename T, typename A>
		static constexpr bool has_attribute()
		{
			return T::template has_attribute<A>();
		}

		template<typename A, typename T>
		static constexpr bool has_attribute(T target)
		{
			return T::template has_attribute<A>();
		}

		template<typename A, typename T>
		static constexpr A get_attribute(T target)
		{
			return T::template get_attribute<A>();
		}

		template<typename T>
		static constexpr const char* get_name(T target)
		{
			return T::template get_name();
		}

	private:
		template<typename T, size_t I, typename F>
		static constexpr void execute(F&& func)
		{
			func(T::template get_field<I>());
		}

		template<typename T, size_t I, typename F>
		static constexpr void execute_in_order(F&& func)
		{
			execute<T, I>(func);
			if constexpr (I + 1 < T::_fieldCount)
				return execute_in_order<T, I + 1>(std::forward<F>(func));
		}
};
