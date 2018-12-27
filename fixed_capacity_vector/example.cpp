#include "test_func.hpp"
#include "nested_struct.hpp"

int main()
{
	test_vector();
	InputMap input_map;
	input_map.get_vec<DType::kInt>().resize(5);
	InternalMap internal_map{ input_map };
}
