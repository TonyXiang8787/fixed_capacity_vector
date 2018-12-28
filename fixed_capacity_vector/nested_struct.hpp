#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>
#include <utility>
#include <vector>
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

template<DType dtype, class = void> struct EnumMap;
template<> struct EnumMap<DType::kD> {
	using type = double;
};
template<> struct EnumMap<DType::kInt8> {
	using type = int8_t;
};
template<> struct EnumMap<DType::kInt16> {
	using type = int16_t;
};
template<DType dtype>
struct EnumMap<dtype,
	std::enable_if_t<dtype == DType::kInt32 || dtype == DType::kInt>>
{
	using type = int32_t;
};
template<DType dtype>
using EnumMapT = typename EnumMap<dtype>::type;

template<DType...dtypes>
struct TypeBase
{
private:
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

public:
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
		bool = typename is_one_of<TNew, TList>::value> struct add_unique;
	template<class TNew, class... UN>
	struct add_unique<TNew, typelist <UN...>, true> {
		using type = typelist <UN...>;
	};
	template<class TNew, typename... UN>
	struct add_unique<TNew, typelist<UN...>, false>
	{
		using type = typelist<TNew, UN...>;
	};

private:

	class Input {
	public:
		template<DType dtype>
		std::vector<EnumMapT<dtype>> & get_vec() {
			return std::get<dtype_index<dtype>>(vectors_);
		}
		template<DType dtype>
		std::vector<EnumMapT<dtype>> const& get_vec() const {
			return std::get<dtype_index<dtype>>(vectors_);
		}
	private:
		std::tuple<std::vector<EnumMapT<dtypes>>...> vectors_;
	};

	class Internal {
	public:
		Internal(Input const& input) :
			vectors_{ input.get_vec<dtypes>().size()... }
		{ }
	private:
		std::tuple<FixedCapacityVector<EnumMapT<dtypes>>...> vectors_;
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
