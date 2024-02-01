#pragma once

#include <type_traits>

template<typename What, typename ... Args>
struct Checker
{
	static constexpr bool value{ (std::is_base_of_v<What, Args> || ...) };
};

class Reflector;

#define REFLECTOR_START(Type)                                           	\
	friend Reflector;                                                   	\
	private:																\
	    using type = Type;                                                  \
	    static constexpr const char* _typeName = #Type;                    	\
	    static constexpr size_t _fieldIndexOffset = __COUNTER__ + 1;       	\
	    template <size_t, typename = void>                                 	\
	    struct Field {};                                                    \
		template<size_t index>												\
		static constexpr Field<index> get_field()							\
		{																	\
			return Field<index>{};											\
		}

#define REFLECTOR_END() \
	static constexpr size_t _fieldCount = __COUNTER__ - _fieldIndexOffset;

#define PROPERTY(Type, Name, ...)									\
	public:															\
		Type Name;													\
																	\
	private:														\
		template<typename __UNUSED>									\
		struct Field<__COUNTER__ - _fieldIndexOffset, __UNUSED>		\
		{															\
			using fieldType = Type;									\
			static constexpr auto pointer{&type::Name};				\
			static constexpr const char* name = #Name;				\
																	\
			template<typename T>									\
			static constexpr bool has_attribute()					\
			{														\
				return Checker<T, __VA_ARGS__>::value;				\
			}														\
		}; 

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

		template<typename A, typename F>
		static constexpr bool has_attribute(F field)
		{
			return F::template has_attribute<A>();
		}

		template<typename F>
		static constexpr const char* get_field_name(F field)
		{
			return F::name;
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
