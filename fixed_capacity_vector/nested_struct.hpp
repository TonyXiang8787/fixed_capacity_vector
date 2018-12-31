#pragma once

#include "fixed_capacity_vector.hpp"
#include <type_traits>
#include <cstddef>
#include <array>
#include <utility>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <limits>
#include <iterator>

using EnumType = int32_t;
using IDType = int32_t;

struct C { int x; };
struct C1 : C {	double y; };
struct C2 : C { int64_t y; };

enum class DType : EnumType {
	kD = 0,
	kInt32 = 100,
	kInt8 = 31,
	kInt = 29,
	kInt16 = -5,
	kC1 = 1000,
	kC2 = -200
};

constexpr std::array<DType, 7> data_type_arr{
	DType::kD,
	DType::kInt32,
	DType::kInt8,
	DType::kInt,
	DType::kInt16,
	DType::kC1,
	DType::kC2
};

template<class T>
struct InputPair {
	int32_t key;
	T value;
};

namespace internal_trait {

// get type by enum
template<DType dtype, class = void> struct enum_map;
template<> struct enum_map<DType::kD> { using type = double; };
template<> struct enum_map<DType::kInt8> { using type = int8_t; };
template<> struct enum_map<DType::kInt16> { using type = int16_t; };
template<> struct enum_map<DType::kC1> { using type = C1; };
template<> struct enum_map<DType::kC2> { using type = C2; };
template<DType dtype>
struct enum_map<dtype,
	std::enable_if_t<dtype == DType::kInt32 || dtype == DType::kInt>> {
	using type = int32_t;
};
template<DType dtype>
using enum_t = typename enum_map<dtype>::type;
template<DType dtype>
using fixed_vector_t = FixedCapacityVector<enum_t<dtype>>;
template<DType dtype>
using input_vector_t = std::vector<InputPair<enum_t<dtype>>>;
template<DType dtype>
using map_t = std::unordered_map<IDType, enum_t<dtype>*>;
template<class T, DType dtype>
constexpr bool is_match = (
	std::is_same<enum_t<dtype>, T>::value ||
	std::is_base_of<T, enum_t<dtype>>::value);

// get index of enum
template <DType d, DType... ds> struct get_index;
template <DType d, DType... ds>
struct get_index<d, d, ds...> : std::integral_constant<size_t, 0> {};
template <DType d, DType d2, DType... ds>
struct get_index<d, d2, ds...> :
	std::integral_constant<std::size_t, 1 + get_index<d, ds...>::value> {};

template<DType...dtypes>
class Input {
public:
	template<DType dtype>
	input_vector_t<dtype> & get_vec() {
		return std::get<get_index<dtype, dtypes...>::value>(vectors_);
	}
	template<DType dtype>
	input_vector_t<dtype> const& get_vec() const {
		return std::get<get_index<dtype, dtypes...>::value>(vectors_);
	}
private:
	std::tuple<input_vector_t<dtypes>...> vectors_;
};

template<DType...dtypes>
class Internal {
public:
	Internal(Input<dtypes...> const& input) :
		vectors_{ input.template get_vec<dtypes>().size()... }
	{
		(build_item<dtypes>(input), ...);
	}


	// get item method
	static constexpr size_t ull_max = std::numeric_limits<size_t>::max();
	template<class T>
	T* get_item(IDType key) {
		size_t index{ 0 }, found_index{ ull_max };
		std::array<T*, sizeof...(dtypes)> found_arr = {
			get_item<T, dtypes>(key, index, found_index)... };
		if (found_index < ull_max) return found_arr[found_index];
		else return nullptr;
	}
	template<DType dtype>
	enum_t<dtype>* get_item(IDType key) {
		using T = enum_t<dtype>;
		size_t index{ 0 }, found_index{ ull_max };
		T* ptr = get_item<T, dtype>(key, index, found_index);
		if (ptr) return ptr;
		else return nullptr;
	}


	// getter for internal map and vector
	template<DType dtype>
	fixed_vector_t<dtype> & get_vec() {
		return std::get<get_index<dtype, dtypes...>::value>(vectors_);
	}
	template<DType dtype>
	map_t<dtype> & get_map() {
		return std::get<get_index<dtype, dtypes...>::value>(maps_);
	}

	template<class T, class Func>
	void for_each(Func func) {
		(for_each<T, dtypes, Func>(func), ...);
	}
private:
	std::tuple<fixed_vector_t<dtypes>...> vectors_;
	std::tuple<map_t<dtypes>...> maps_;

	// build, get, for_each, per category
	template<DType dtype>
	void build_item(Input<dtypes...> const& input) {
		using T = enum_t<dtype>;
		using TVector = fixed_vector_t<dtype>;
		using TMap = map_t<dtype>;
		input_vector_t<dtype> const& input_vec =
			input.template get_vec<dtype>();
		TVector& internal_vec = get_vec<dtype>();
		TMap& internal_map = get_map<dtype>();
		for (auto & input_pair : input_vec) {
			internal_map.insert(
				{ input_pair.key,
				&(internal_vec.emplace_back(input_pair.value)) }
			);
		}
	}
	template<class T, DType dtype>
	T* get_item(IDType key, size_t& index, size_t& found_index) {
		index++;
		if (found_index < ull_max) return nullptr;
		if constexpr (is_match<T, dtype>) {
			auto & comp_map = get_map<dtype>();
			auto iter = comp_map.find(key);
			if (iter != comp_map.end()) {
				found_index = index - 1;
				return iter->second;
			}
		}
		return nullptr;
	}
	template<class T, DType dtype, class Func>
	void for_each(Func func) {
		if constexpr (is_match<T, dtype>) {
			fixed_vector_t<dtype>& vec = get_vec<dtype>();
			for (T& item : vec)
				func(item);
		}
	}

public:
	// iterator
	template<class T>
	class Iterator {
	private:
		using PtrPair = std::pair<void*, void*>;
		using PtrDeref = T * (Iterator::*)();
		static constexpr std::array<bool, sizeof...(dtypes)> enable_vec_{
			is_match<T, dtypes>...};
		template<DType dtype>
		T* deref() {
			if constexpr (is_match<T, dtype>) return (enum_t<dtype>*)ptr_;
			else return nullptr;
		}
		static constexpr std::array<PtrDeref, sizeof...(dtypes)> deref_func_{
			&deref<dtypes>...};
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;
	private:
		std::array<PtrPair, sizeof...(dtypes)> const ptr_pairs_{};
		size_t seq_{ 0 };  // sequence number
		void* ptr_{ nullptr };  // void pointer

		
	};
};


template<DType...dtypes>
struct TypeBase
{
private:
	template<DType d>
	static constexpr size_t dtype_index =
		get_index<d, dtypes...>::value;

public:
	using InputMap = Input<dtypes...>;
	using InternalMap = Internal<dtypes...>;
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
