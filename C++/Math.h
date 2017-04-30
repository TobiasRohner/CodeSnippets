#pragma once

#include <limits>
#include <cassert>


//A compile-time abs function
template<typename T>
constexpr T cabs(T n)
{
	return n < 0 ? -n : n;
}


//Check for power of two
template<typename T>
constexpr bool isPower2(T n)
{
	assert(std::numeric_limits<T>::is_integer);
	return !((n - 1) & n);
}
