#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>

enum class DType : int32_t {
	kD = 0,
	kInt32,
	kInt8,
	kInt,
	kInt16
};
constexpr std::array<DType, 5> data_type_arr {
	DType::kD,
	DType::kInt32,
	DType::kInt8,
	DType::kInt,
	DType::kInt16
};

template<DType dtype, class = void> struct enum_map;
template<> struct enum_map<DType::kD> {
	using type = double; };
template<> struct enum_map<DType::kInt8> {
	using type = int8_t; };
template<> struct enum_map<DType::kInt16> {
	using type = int16_t; };
template<DType dtype> 
struct enum_map<dtype, 
	std::enable_if_t<dtype == DType::kInt32 || dtype == DType::kInt>>
{
	using type = int32_t;
};

template<size_t N, class = void> struct v_size;
template<size_t N> 
struct v_size<N, std::enable_if_t<N == 0>> { size_t size; };
template<size_t N>
struct v_size<N, std::enable_if_t<(N > 0)>> : v_size<N - 1> {
	size_t size;
};
using v_size_t = v_size<data_type_arr.size() - 1>;
