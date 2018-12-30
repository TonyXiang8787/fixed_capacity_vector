#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <climits>

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

namespace internal_trait {

// get type by enum
template<DType dtype, class = void> struct enum_map;
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

template<DType...dtypes>
struct TypeBase
{
private:
	template<DType d>
	static constexpr size_t dtype_index =
		get_index<d, dtypes...>::value;

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

	class Internal {
	public:
		Internal(Input const& input) :
			vectors_{ input.template get_vec<dtypes>().size()... }
		{ }

		template<class T>
		T* get_item(int32_t key) {
			size_t index{ 0 }, found_index{ ULLONG_MAX };
			std::array<T*, sizeof...(dtypes)> found_arr = { 
				get_item<T, dtypes>(key, index, found_index)... };
			if (found_index < ULLONG_MAX) return found_arr[found_index];
			else return nullptr;
		}
		template<DType dtype>
		enum_map_t<dtype>* get_item(int32_t key) {
			using T = enum_map_t<dtype>;
			size_t index{ 0 }, found_index{ ULLONG_MAX };
			T* ptr = get_item<T, dtype>(key, index, found_index);
			if (ptr) return ptr;
			else return nullptr;
		}
		template<class T, DType dtype>
		T* get_item(int32_t key, size_t& index, size_t& found_index) { 
			index++;
			if (found_index < ULLONG_MAX) return nullptr;
			if constexpr (
				std::is_same<enum_map_t<dtype>, T>::value ||
				std::is_base_of<enum_map_t<dtype>, T>::value) {
				auto & comp_map = std::get<dtype_index<dtype>>(maps_);
				auto iter = comp_map.find(key);
				if (iter != comp_map.end())	{
					found_index = index - 1;
					return iter->second; 
				}
			}
			return nullptr;
		}

	private:
		std::tuple<FixedCapacityVector<enum_map_t<dtypes>>...> vectors_;
		std::tuple<std::unordered_map<int32_t, enum_map_t<dtypes>*>...> maps_;
	};
public:
	using InputMap = Input;
	using InternalMap = Internal;

	static void resize_input(InputMap & input_map, size_t size) {
		(input_map.template get_vec<dtypes>().resize(size), ...);
	}
};

template<class> struct TypeTrait;
template<size_t...Ints>
struct TypeTrait<std::index_sequence<Ints...>> {
	using TypeBaseT = TypeBase<data_type_arr[Ints]...>;
};

}

using Root = internal_trait::TypeTrait<
	std::make_index_sequence<data_type_arr.size()>>::TypeBaseT;
using InputMap = Root::InputMap;
using InternalMap = Root::InternalMap;
