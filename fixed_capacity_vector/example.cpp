#include "test_func.hpp"
#include "nested_struct.hpp"

int main()
{
	test_vector();
	VSizeT v_size;
	((VSize<DType::kInt8>&)v_size).size = 5;
}
