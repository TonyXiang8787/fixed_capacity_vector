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
// assert of dtypes satisfies
template<bool...> struct bool_pack;
template<bool... bs>
using all_true = std::is_same<
	bool_pack<bs..., true>, 
	bool_pack<true, bs...>>;
template<class T, DType...ds>
constexpr bool type_check = all_true<is_match<T, ds>...>::value;
// seq of dtypes
template<DType...ds>
using dtype_seq = std::integer_sequence<DType, ds...>;
// add match dtypes
template<class DSeq, class T, DType d, bool = is_match<T, d>> 
struct add_match;
template<DType...ds, class T, DType d>
struct add_match<dtype_seq<ds...>, T, d, true> {
	using type = dtype_seq<ds..., d>;
};
template<DType...ds, class T, DType d>
struct add_match<dtype_seq<ds...>, T, d, false> {
	using type = dtype_seq<ds...>;
};
// get match dtypes
template<class T, class DList1, class DList2 = dtype_seq<>> 
struct matched_dtypes;
template<class T, DType...d2>
struct matched_dtypes<T, dtype_seq<>, dtype_seq<d2...>> {
	using type = dtype_seq<d2...>;
};
template<class T, DType d0, DType...d1, DType...d2>
struct matched_dtypes<T, dtype_seq<d0, d1...>, dtype_seq<d2...>> {
	using type = typename matched_dtypes<
		T,
		dtype_seq<d1...>,
		typename add_match<dtype_seq<d2...>, T, d0>::type
	>::type;
};
// get iterator type
template<template<class, DType...> class IteratorClass,	class T, class DSeq>
struct get_iterator_type;
template<template<class, DType...> class IteratorClass, class T, DType...ds>
struct get_iterator_type<IteratorClass, T, dtype_seq<ds...>> {
	using type = IteratorClass<T, ds...>;
};
template<template<class, DType...> class IteratorClass, class T, DType...ds>
struct filter_iterator_type {
	using filter_seq = typename matched_dtypes<T, dtype_seq<ds...>>::type;
	using type = typename get_iterator_type<IteratorClass, T, filter_seq>::type;
};

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
	static constexpr size_t n_types = sizeof...(dtypes);

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
		std::array<T*, n_types> found_arr = {
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
private:
	std::tuple<fixed_vector_t<dtypes>...> vectors_;
	std::tuple<map_t<dtypes>...> maps_;
	// build, get, per category
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
private:
	// iterator
	template<class T, DType...ds>
	class Iterator {
	private:
		static_assert(type_check<T, ds...>);
		static constexpr size_t n_ds = sizeof...(ds);
		static_assert(n_ds > 0);
		using PtrPair = std::pair<char*, char*>;
		inline static const std::array<size_t, n_ds> offset_{
			reinterpret_cast<size_t>(static_cast<T*>(
				reinterpret_cast<enum_t<ds>*>(0)
				))...};
		static constexpr std::array<size_t, n_ds> incr_{
			sizeof(enum_t<ds>)... };
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using difference_type = std::ptrdiff_t;
		using pointer = T*;
		using reference = T&;
		Iterator() : ptr_pairs_{} {}
		Iterator(Internal& internal, bool is_begin):
			ptr_pairs_{ get_range<ds>(internal)... }
		{
			if (is_begin) {
				for (; seq_ < n_ds; seq_++) {
					if (ptr_pairs_[seq_].first) {
						ptr_ = ptr_pairs_[seq_].first;
						return;
					}
				}
			}
			else {
				seq_ = n_ds;
			}
		}
		Iterator& operator++() {
			ptr_ = ptr_ + incr_[seq_];
			if (ptr_ == ptr_pairs_[seq_].second)
			{
				ptr_ = nullptr;
				seq_++;
				for (; seq_ < n_ds; seq_++) {
					if (ptr_pairs_[seq_].first) {
						ptr_ = ptr_pairs_[seq_].first;
						break;
					}
				}
			}
			return *this;
		}
		Iterator operator++(int) {
			Iterator iter(*this);
			++(*this);
			return iter;
		}
		reference operator* () const { 
			return *reinterpret_cast<T*>(ptr_ + offset_[seq_]); }
		pointer operator->() const { 
			return reinterpret_cast<T*>(ptr_ + offset_[seq_]); }
		bool operator==(Iterator const& rhs) const { 
			return (ptr_ == rhs.ptr_) && (seq_ == rhs.seq_); }
		bool operator!=(Iterator const& rhs) const {
			return !(*this == rhs);	}
	private:
		std::array<PtrPair, n_ds> const ptr_pairs_;
		size_t seq_{ 0 };  // sequence number
		char* ptr_{ nullptr };  // void pointer

		template<DType dtype>
		PtrPair get_range(Internal& internal) {
			if (internal.get_vec<dtype>().size() > 0)
				return {
				reinterpret_cast<char*>(internal.get_vec<dtype>().begin()),
				reinterpret_cast<char*>(internal.get_vec<dtype>().end()) 
			};
			return { nullptr, nullptr };
		}
	};
	// proxy object
	template<class T>
	class Proxy {
	private:
		using IteratorType = typename filter_iterator_type<
			Iterator, T, dtypes...>::type;
	public:
		Proxy(Internal& internal):
			begin_{ internal, true },
			end_{ internal, false }
		{}
		IteratorType begin() { return begin_; }
		IteratorType end() { return end_; }
	private:
		IteratorType const begin_;
		IteratorType const end_;
	};
public:
	template<class T>
	Proxy<T> iter() {
		return Proxy<T>{ *this };
	}
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
