#include "test_func.hpp"
#include "nested_struct.hpp"

int main()
{
	test_vector();
	InputMap input_map;
	Root::resize_input(input_map, 5);
	InternalMap internal_map{ input_map };

	static_assert(std::is_same_v<
		Root::typelist<int, double, float>,
		Root::unique_type_list<Root::typelist<int, double, double, float, int, double>>::type
	>);
}
