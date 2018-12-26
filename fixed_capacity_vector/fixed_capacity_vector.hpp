#pragma once

#include <memory>


template<class T>
class FixedCapacityVector {
private:
	using StorageType = std::aligned_storage_t<sizeof(T), alignof(T)>;
public:
	FixedCapacityVector(FixedCapacityVector const&) = delete;
	FixedCapacityVector& operator=(FixedCapacityVector const&) = delete;
	FixedCapacityVector(FixedCapacityVector &&) = delete;
	FixedCapacityVector& operator=(FixedCapacityVector &&) = delete;

private:
	std::unique_ptr<T[]> data_;
	size_t size_;
	size_t const capacity_;
};