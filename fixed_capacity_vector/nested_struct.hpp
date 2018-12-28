#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>

using EnumType = int32_t;

enum class DType : EnumType {
	kD = 0,
	kInt32 = 100,
	kInt8 = 31,
	kInt = 29,
	kInt16 = -5
};

constexpr std::array<DType, 5> data_type_arr {
	DType::kD,
	DType::kInt32,
	DType::kInt8,
	DType::kInt,
	DType::kInt16
};



template<DType...dtypes>
struct TypeBase
{
private:
	// get type by enum
	template<DType dtype, class = void> struct enum_map { };
	template<> struct enum_map<DType::kD> { using type = double; };
	template<> struct enum_map<DType::kInt8> { using type = int8_t; };
	template<> struct enum_map<DType::kInt16> { using type = int16_t; };
	template<DType dtype>
	struct enum_map<dtype,
		std::enable_if_t<dtype == DType::kInt32 || dtype == DType::kInt>> {
		using type = int32_t;
	};
	template<DType dtype>
	using enum_map_t = typename enum_map<dtype>::type;
	// get index of enum
	template <DType d, DType... ds> struct get_index;
	template <DType d, DType... ds>
	struct get_index<d, d, ds...> : std::integral_constant<size_t, 0> {};
	template <DType d, DType d2, DType... ds>
	struct get_index<d, d2, ds...> :
		std::integral_constant<std::size_t, 1 + get_index<d, ds...>::value> {};
	template<DType d>
	static constexpr size_t dtype_index = 
		get_index<d, dtypes...>::value;
	// is one of types
	template<class...> struct typelist;
	template<class T, class TList> struct is_one_of;
	template<class T> 
	struct is_one_of<T, typelist<>> : std::false_type { };
	template<class T, class... UN> 
	struct is_one_of<T, typelist<T, UN...>> : std::true_type { };
	template<class T, class U0, class... UN>
	struct is_one_of<T, typelist<U0, UN...>> : is_one_of<T, typelist<UN...>> { };
	// add if unique
	template<class TNew, class TList, 
		bool = is_one_of<TNew, TList>::value> struct add_unique { };
	template<class TNew, class... UN>
	struct add_unique<TNew, typelist <UN...>, true> {
		using type = typelist<UN...>; 
	};
	template<class TNew, class... UN>
	struct add_unique<TNew, typelist<UN...>, false> {
		using type = typelist<UN..., TNew>; 
	};
	// calculate unique types
	template<class TList1, class TList2 = typelist<>> struct unique_type_list {};
	template<class... TR>
	struct unique_type_list<typelist<>, typelist<TR...>> {
		using type = typelist<TR...>;
	};
	template<class T0, class... TN, class... TR>
	struct unique_type_list<typelist<T0, TN...>, typelist<TR...>> {
		using type = typename unique_type_list<
			typelist<TN...>, 
			typename add_unique<T0, typelist<TR...>>::type
		>::type;
	};

	class Input {
	public:
		template<DType dtype>
		std::vector<enum_map_t<dtype>> & get_vec() {
			return std::get<dtype_index<dtype>>(vectors_);
		}
		template<DType dtype>
		std::vector<enum_map_t<dtype>> const& get_vec() const {
			return std::get<dtype_index<dtype>>(vectors_);
		}
	private:
		std::tuple<std::vector<enum_map_t<dtypes>>...> vectors_;
	};


	// get map class
	template<class...> struct map_traits;
	template<class... T> struct map_traits<typelist<T...>> {
		using type = std::tuple<std::unordered_map<int32_t, T>...>;
	};
	using ComponentMaps = typename map_traits<
		typename unique_type_list<typelist<enum_map_t<dtypes>...>>::type>::type;

	class Internal {
	public:
		Internal(Input const& input) :
			vectors_{ input.get_vec<dtypes>().size()... }
		{ }
	private:
		std::tuple<FixedCapacityVector<enum_map_t<dtypes>>...> vectors_;
		ComponentMaps maps_;
	};
public:
	using InputMap = Input;
	using InternalMap = Internal;

	static void resize_input(InputMap & input_map, size_t size) {
		(input_map.get_vec<dtypes>().resize(size), ... );
	}
};

template<class> struct TypeTrait;
template<size_t...Ints>
struct TypeTrait<std::index_sequence<Ints...>> {
	using TypeBaseT = TypeBase<data_type_arr[Ints]...>;
};

using Root = typename TypeTrait<
	std::make_index_sequence<data_type_arr.size()>>::TypeBaseT;
using InputMap = Root::InputMap;
using InternalMap = Root::InternalMap;
