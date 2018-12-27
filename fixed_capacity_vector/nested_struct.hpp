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

template<class> struct TypeTrait;
template<size_t...Ints>
struct TypeTrait<std::index_sequence<Ints...>>
{
private:
	template <DType dtype, DType... dtypes> struct get_index;
	template <DType dtype, DType... dtypes>
	struct get_index<dtype, dtype, dtypes...> : 
		std::integral_constant<size_t, 0> {};
	template <DType dtype, DType dtype_other, DType... dtypes>
	struct get_index<dtype, dtype_other, dtypes...> :
		std::integral_constant<std::size_t, 
		1 + get_index<dtype, dtypes...>::value> {};
	template<DType dtype>
	static constexpr size_t dtype_index = 
		get_index<dtype, data_type_arr[Ints]...>::value;

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
		std::tuple<std::vector<EnumMapT<data_type_arr[Ints]>>...> vectors_;
	};

	class Internal {
	public:
		Internal(Input const& input) :
			vectors_{ input.get_vec<data_type_arr[Ints]>().size()... }
		{ }
	private:
		std::tuple<FixedCapacityVector<EnumMapT<data_type_arr[Ints]>>...> vectors_;
	};
public:
	using InputMap = Input;
	using InternalMap = Internal;
};

using InputMap = typename TypeTrait<
	std::make_index_sequence<data_type_arr.size()>>::InputMap;
using InternalMap = typename TypeTrait<
	std::make_index_sequence<data_type_arr.size()>>::InternalMap;
