#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>
#include <utility>

using EnumType = int32_t;

enum class DType : EnumType {
	kD = 0,
	kInt32,
	kInt8,
	kInt,
	kInt16
};
inline constexpr DType prev(DType dtype) {
	return static_cast<DType>(
		static_cast<EnumType>(dtype) - 1); }
inline constexpr DType next(DType dtype) {
	return static_cast<DType>(
		static_cast<EnumType>(dtype) + 1);
}

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

template<DType dtype, class = void> struct VSize;
template<DType dtype>
struct VSize<dtype, std::enable_if_t<dtype == data_type_arr.front()>> { 
	size_t size_; };
template<DType dtype>
struct VSize<dtype, std::enable_if_t<dtype != data_type_arr.front()>> : 
	VSize<prev(dtype)> { size_t size_; };
struct VSizeT : VSize<data_type_arr.back()> {
	template<DType dtype>
	size_t& size() { return static_cast<VSize<dtype> *>(this)->size_; }
};

template <DType...>
struct haha {};

template<class> struct hoho;
template<EnumType...Ints>
struct hoho<std::integer_sequence<EnumType, Ints...>>
{
	using haha_t = haha<static_cast<DType>(Ints)...>;
};

using hoho_t = typename hoho<std::make_integer_sequence<EnumType, data_type_arr.size()>>::haha_t;
