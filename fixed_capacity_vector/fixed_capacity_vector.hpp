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

	FixedCapacityVector(size_t capacity = 0):
		capacity_{ capacity },
		data_{ std::make_unique<StorageType[]>(capacity) }
	{ }

	template<class... Args>
	T& emplace_back(Args&&... args)
	{
		new (&data_[size_]) T{ std::forward<Args>(args)... };
	}
private:
	size_t const capacity_;
	std::unique_ptr<StorageType[]> const data_;
	size_t size_{ 0 };
};