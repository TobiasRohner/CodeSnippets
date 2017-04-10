#pragma once

#include <cstdint>
#include <utility>
#include <ostream>



/*
This Array Class dynamically allocates its memory at runtime.
After initialization, the size of the array cannot be modified
*/
template<typename T>
class Array
{
public:
	typedef T value_type;

	Array();
	Array(size_t s);
	Array(const Array<T>& other);
	Array(Array&& other);
	~Array();

	Array& operator=(const Array<T>& other);
	Array& operator=(Array<T>&& other);
	T& operator[](size_t i);
	const T& operator[](size_t i) const;
	Array<T>& operator+=(const Array<T>& other);
	template<typename T2> friend Array<T2>& operator*=(size_t n, Array<T2>& arr);
	Array<T> operator+(const Array<T>& other) const;
	template<typename T2> friend Array<T2> operator*(size_t n, const Array<T2>& arr);
	template<typename T2> friend std::ostream& operator<<(std::ostream& os, const Array<T2>& arr);

	T* begin();
	const T* begin() const;
	T* end();
	const T* end() const;

	size_t size() const;


private:
	T* data = nullptr;
	size_t array_size = 0;

	void swap(Array& other);
};


template<typename T>
Array<T>::Array() : data(nullptr), array_size(0)
{
}


template<typename T>
Array<T>::Array(size_t s) : data(new T[s]), array_size(s)
{
}


template<typename T>
Array<T>::Array(const Array<T>& other) : data(new T[other.array_size]), array_size(other.array_size)
{
	for (size_t i = 0; i < array_size; ++i)
		data[i] = other[i];
}


template<typename T>
Array<T>::Array(Array<T>&& other)
{
	swap(other);
}


template<typename T>
Array<T>::~Array()
{
	if (data)
		delete[] data;
}


template<typename T>
Array<T>& Array<T>::operator=(const Array<T>& other)
{
	if (data)
		delete[] data;
	array_size = other.array_size;
	data = new T[array_size];
	for (size_t i = 0; i < array_size; ++i)
		data[i] = other[i];
	return *this;
}


template<typename T>
Array<T>& Array<T>::operator=(Array<T>&& other)
{
	swap(other);
	return *this;
}


template<typename T>
T& Array<T>::operator[](size_t i)
{
	return data[i];
}


template<typename T>
const T& Array<T>::operator[](size_t i) const
{
	return data[i];
}


template<typename T>
Array<T>& Array<T>::operator+=(const Array<T>& other)
{
	swap(*this + other);
	return *this;
}


template<typename T2>
Array<T2>& operator*=(size_t n, Array<T2>& arr)
{
	arr.swap(n * arr);
	return arr;
}


template<typename T>
Array<T> Array<T>::operator+(const Array<T>& other) const
{
	Array<T> result(array_size + other.array_size);
	for (size_t i = 0; i < array_size; ++i)
		result[i] = data[i];
	for (size_t i = array_size; i < array_size + other.array_size; ++i)
		result[i] = other[i];
	return result;
}


template<typename T2>
Array<T2> operator*(size_t n, const Array<T2>& arr)
{
	Array<T2> result(n * arr.array_size);
	for (size_t i = 0; i < n * arr.array_size; ++i)
		result[i] = arr[i % arr.array_size];
	return result;
}


template<typename T2>
std::ostream& operator<<(std::ostream& os, const Array<T2>& arr)
{
	os << "[";
	if (arr.array_size >= 1)
		os << arr[0];
	for (auto it = arr.begin() + 1; it != arr.end(); ++it)
		os << ", " << *it;
	os << "]";
	return os;
}


template<typename T>
T* Array<T>::begin()
{
	return data;
}


template<typename T>
const T* Array<T>::begin() const
{
	return data;
}


template<typename T>
T* Array<T>::end()
{
	return data + array_size;
}


template<typename T>
const T* Array<T>::end() const
{
	return data + array_size;
}


template<typename T>
size_t Array<T>::size() const
{
	return array_size;
}


template<typename T>
void Array<T>::swap(Array<T>& other)
{
	std::swap(array_size, other.array_size);
	std::swap(data, other.data);
}