#pragma once

#include "reflection.h"
#include <type_traits>

template<typename What, typename ...Args>
struct Checker
{
	static constexpr bool value{ (std::is_base_of_v<What, Args> || ...) };
};

class Reflector;

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
		template<typename T>												\
		static constexpr bool has_attribute()								\
		{																	\
			return Checker<T, __VA_ARGS__>::value;							\
		}																	\
																			\
		static constexpr const char* get_name()								\
		{																	\
			return #Type;													\
		}

#define DEFINE_META_METHODS_PLACEHOLDERS()									\
	template<typename T>													\
	static constexpr bool has_meta() { return false; }						\
	static constexpr void* get_meta() { return nullptr; }					\

#define DEFINE_META_METHODS(Meta)											\
	static constexpr decltype(Meta) meta = Meta;							\
																			\
	template<typename M>													\
	static constexpr bool has_meta()										\
	{																		\
		return std::is_same_v<M, decltype(Meta)>;							\
	}																		\
																			\
	static constexpr const decltype(Meta)* get_meta()						\
	{																		\
		return &meta;														\
	}																		\

#define REFLECTOR_START(Type, ...)											\
	REFLECT_TYPE_BASE(Type, __VA_ARGS__)									\
	DEFINE_META_METHODS_PLACEHOLDERS()

#define REFLECTOR_START_M(Type, Meta, ...)									\
	REFLECT_TYPE_BASE(Type, __VA_ARGS__)									\
	DEFINE_META_METHODS(Meta)												\

#define REFLECTOR_END()														\
	static constexpr size_t _fieldCount = __COUNTER__ - _fieldIndexOffset;

#define REFLECT_FIELD_BASE(Type, Name, ...)									\
	using fieldType = Type;													\
	static constexpr auto pointer{&type::Name};								\
																			\
	template<typename A>													\
	static constexpr bool has_attribute()									\
	{																		\
		return Checker<A, __VA_ARGS__>::value;								\
	}																		\
																			\
	static constexpr const char* get_name()									\
	{																		\
		return #Name;														\
	}

#define REFLECT_FIELD(Type, Name, ...)										\
	private:																\
		template<typename __UNUSED>											\
		struct Field<__COUNTER__ - _fieldIndexOffset, __UNUSED>				\
		{																	\
			REFLECT_FIELD_BASE(Type, Name, __VA_ARGS__)						\
			DEFINE_META_METHODS_PLACEHOLDERS()								\
		};

#define REFLECT_FIELD_M(Type, Name, Meta, ...)								\
	private:																\
	template<typename __UNUSED>												\
	struct Field<__COUNTER__ - _fieldIndexOffset, __UNUSED>					\
	{																		\
		REFLECT_FIELD_BASE(Type, Name, __VA_ARGS__)							\
		DEFINE_META_METHODS(Meta)											\
	};

#define FIELD(Type, Name, ...)												\
	public:																	\
		Type Name;															\
	REFLECT_FIELD(Type, Name, __VA_ARGS__)

#define FIELD_M(Type, Name, Meta, ...)										\
	public:																	\
		Type Name;															\
	REFLECT_FIELD_M(Type, Name, Meta, __VA_ARGS__)

#define PRIVATE_PROPERTY(Type, Name, ...)									\
	private:																\
		Type Name;															\
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

		template<typename T>
		static constexpr const char* get_name(T target)
		{
			return T::template get_name();
		}

		template<typename T, typename M>
		static constexpr bool has_meta()
		{
			return T::template has_meta<M>();
		}

		template<typename M, typename T>
		static constexpr bool has_meta(T target)
		{
			return T::template has_meta<M>();
		}

		template<typename T>
		static constexpr decltype(auto) get_meta(T target = {})
		{
			return T::template get_meta();
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
