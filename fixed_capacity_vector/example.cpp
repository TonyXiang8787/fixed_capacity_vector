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
	Root::resize_input(input_map, 5);
	InternalMap internal_map{ input_map };
	int * found = internal_map.get_item<int>(5);

	std::cout << "***********Test compile time struct*********\n";
	std::cout << found << '\n';
}

int main()
{
	test_vector();
	test_struct();
}


