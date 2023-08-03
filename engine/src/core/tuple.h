#pragma once

#include <tuple>

namespace ad_astris
{
	template<typename ...ARGS>
	class Tuple
	{
		public:
			Tuple() = default;

			Tuple(ARGS... args) : _tuple(args...)
			{
				
			}
		
			// Tuple(ARGS&&... args) : _tuple(std::forward<ARGS>(args)...)
			// {
			// 	
			// }

			template<typename T>
			T& get()
			{
				return std::get<T>(_tuple);
			}

		private:
			std::tuple<ARGS...> _tuple;
	};

	namespace tuple_spec
	{
		template<std::size_t N>
		struct CustomDataTypeToTuple;

		// template<>
		// struct CustomDataTypeToTuple<0>
		// {
		// 	template<class S>
		// 	auto operator()(S&& s)const {
		// 		auto[e0]=std::forward<S>(s);
		// 		return std::make_tuple(e0);
		// 	}
		// };

		template<>
		struct CustomDataTypeToTuple<1>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0]=std::forward<S>(s);
				return std::make_tuple(e0);
			}
		};

		template<>
		struct CustomDataTypeToTuple<2>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1]=std::forward<S>(s);
				return std::make_tuple(e0, e1);
			}
		};

		template<>
		struct CustomDataTypeToTuple<3>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2);
			}
		};

		template<>
		struct CustomDataTypeToTuple<4>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3);
			}
		};

		template<>
		struct CustomDataTypeToTuple<5>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4);
			}
		};

		template<>
		struct CustomDataTypeToTuple<6>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5);
			}
		};
		
		template<>
		struct CustomDataTypeToTuple<7>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5, e6]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5, e6);
			}
		};

		template<>
		struct CustomDataTypeToTuple<8>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5, e6, e7]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5, e6, e7);
			}
		};

		template<>
		struct CustomDataTypeToTuple<9>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5, e6, e7, e8]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5, e6, e7, e8);
			}
		};

		template<>
		struct CustomDataTypeToTuple<10>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5, e6, e7, e8, e9]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9);
			}
		};

		template<>
		struct CustomDataTypeToTuple<11>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10);
			}
		};
		
		template<>
		struct CustomDataTypeToTuple<12>
		{
			template<class S>
			auto operator()(S&& s)const {
				auto[e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11]=std::forward<S>(s);
				return std::make_tuple(e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11);
			}
		};
	}
}