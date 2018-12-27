#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>
#include <utility>
#include <vector>

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
	using type = double; };
template<> struct EnumMap<DType::kInt8> {
	using type = int8_t; };
template<> struct EnumMap<DType::kInt16> {
	using type = int16_t; };
template<DType dtype> 
struct EnumMap<dtype, 
	std::enable_if_t<dtype == DType::kInt32 || dtype == DType::kInt>>
{
	using type = int32_t;
};
template<DType dtype>
using EnumMapT = typename EnumMap<dtype>::type;

template <DType dtype>
struct InputVec {
	std::vector<EnumMapT<dtype>> vec;
};

template <DType...dtypes>
class Input {
public:
	template<DType dtype>
	std::vector<EnumMapT<dtype>> & get_vec() {
		return std::get<InputVec<dtype>>(vectors_).vec;
	}
	template<DType dtype>
	std::vector<EnumMapT<dtype>> const& get_vec() const {
		return std::get<InputVec<dtype>>(vectors_).vec;
	}
private:
	std::tuple<InputVec<dtypes>...> vectors_;
};

template <DType dtype>
struct InternalVec {
	FixedCapacityVector<EnumMapT<dtype>> vec;
};

template <DType...dtypes>
class Internal {
public:
	Internal(Input<dtypes...> const& input) :
		vectors_{ input.get_vec<dtypes>().size()... }
	{ }
private:
	std::tuple<FixedCapacityVector<EnumMapT<dtypes>>...> vectors_;
};

template<class> struct TypeTrait;
template<size_t...Ints>
struct TypeTrait<std::index_sequence<Ints...>>
{
	using InputMap = Input<data_type_arr[Ints]...>;
	using InternalMap = Internal<data_type_arr[Ints]...>;
};

using InputMap = typename TypeTrait<std::make_index_sequence<data_type_arr.size()>>::InputMap;
using InternalMap = typename TypeTrait<std::make_index_sequence<data_type_arr.size()>>::InternalMap;
