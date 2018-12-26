#pragma once

#include <memory>

template<class T>
class FixedCapacityVector {
private:
	using StorageType = std::aligned_storage_t<sizeof(T), alignof(T)>;
	static_assert(sizeof(StorageType) == sizeof(T));
public:
	FixedCapacityVector(FixedCapacityVector const&) = delete;
	FixedCapacityVector& operator=(FixedCapacityVector const&) = delete;
	FixedCapacityVector(size_t capacity = 0):
		capacity_{ capacity },
		data_{ std::make_unique<StorageType[]>(capacity) }
	{ }
	~FixedCapacityVector()
	{
		for (size_t i = 0; i < size_; i++)
			reinterpret_cast<T&>(data_[i]).~T();
	}
	template<class... Args>
	T& emplace_back(Args&&... args) 
	{
		if (size_ == capacity_)
			throw std::bad_alloc{};
		new (&data_[size_]) T{ std::forward<Args>(args)... };
		return reinterpret_cast<T&>(data_[size_++]);
	}
	T& operator[](size_t i) 
	{ return reinterpret_cast<T&>(data_[i]); }
	T const& operator[](size_t i) const 
	{ return reinterpret_cast<T const&>(data_[i]); }
	size_t size() const { return size_; }
	size_t capacity() const { return capacity_; }
	T* data() { return reinterpret_cast<T*>(data_.get()); }
	T const* data() const { return reinterpret_cast<T const*>(data_.get()); }
	T* begin() { return data(); }
	T* end() { return data() + size_; }
	T const* begin() const { return data(); }
	T const* end() const { return data() + size_; }
private:
	size_t const capacity_;
	std::unique_ptr<StorageType[]> const data_;
	size_t size_{ 0 };
};