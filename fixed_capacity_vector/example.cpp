#include "nested_struct.hpp"
#include "fixed_capacity_vector.hpp"
#include <iostream>

void test_vector()
{
	struct C {
		C(C const&) = delete;
		C& operator=(C const&) = delete;

		C(int x, double y) : a{ x }, b{ y }
		{}
		~C() {
			std::cout << a << ", " << b << " destroyed!\n";
		}
		int a;
		double b;
		char c{ 'x' };
	};

	std::cout << "***********Test vector*********\n";
	FixedCapacityVector<int> vec1{ 1 };
	vec1.emplace_back(5);
	try {
		vec1.emplace_back(6);
	}
	catch (std::bad_alloc &) {
		std::cout << "Capacity reached limit\n";
	}

	FixedCapacityVector<C> vec2{ 3 };
	vec2.emplace_back(2, 5.0).c = 'o';
	vec2.emplace_back(10, 9.0).c = 'z';
	std::cout << vec2.size() << '\n';
	std::cout << vec2.capacity() << '\n';
	for (C & item : vec2)
	{
		std::cout << item.c << '\n';
	}
	auto const& vec3 = vec2;
	for (C const & item : vec3)
	{
		std::cout << item.b << '\n';
	}
	for (auto it = vec2.cbegin(); it != vec2.cend(); it++)
	{
		C const& item = *it;
		std::cout << item.a << '\n';
	}
}


void test_struct() {
	InputMap input_map;
	input_map.get_vec<DType::kD>() = {
		{0, 100.0},
		{1, 2.0}
	};
	input_map.get_vec<DType::kInt>() = {
		{2, 30},
		{3, 50}
	};
	input_map.get_vec<DType::kInt32>() = {
		{4, -5},
		{5, -21}
	};
	input_map.get_vec<DType::kC1>() = {
		{6, { {200}, -5.0} },
		{7, { {300}, -10.0}}
	};
	input_map.get_vec<DType::kC2>() = {
		{8, { {400}, 1000} },
		{9, { {500}, 2000}}
	};
	InternalMap internal_map{ input_map };
	internal_map.build_item(input_map);

	std::cout << "***********Test compile time struct*********\n";
	std::cout << *(internal_map.get_item<int>(5)) << '\n';
	std::cout << *(internal_map.get_item<double>(0)) << '\n';
	std::cout << *(internal_map.get_item<DType::kInt>(2)) << '\n';
	std::cout << internal_map.get_item<double>(5) << '\n';
	std::cout << internal_map.get_item<DType::kInt>(5) << '\n';
	std::cout << internal_map.get_item<C>(6)->x << '\n';
	std::cout << internal_map.get_item<C>(7)->x << '\n';
	std::cout << internal_map.get_item<C>(8)->x << '\n';
	std::cout << internal_map.get_item<C>(9)->x << '\n';
	std::cout << internal_map.get_item<C2>(8)->y << '\n';
	std::cout << internal_map.get_item<C2>(9)->y << '\n';

	using IT = InternalMap::Iterator<C>;
	auto x = IT{ 1,2 };
	auto y = IT{ 2,3 };
	std::cout << (x == y) << '\n';
	std::cout << (x != y) << '\n';
}

int main()
{
	test_vector();
	test_struct();
}


