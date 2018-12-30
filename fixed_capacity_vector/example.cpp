#include "test_func.hpp"
#include "nested_struct.hpp"

int main()
{
	test_vector();
	InputMap input_map;
	Root::resize_input(input_map, 5);
	InternalMap internal_map{ input_map };
	int * found = internal_map.get_item<int>(5);
}
