#pragma once

#include <limits>
#include <cassert>


//A compile-time abs function
template<typename T>
inline constexpr T cabs(T n)
{
	return n < 0 ? -n : n;
}


//Check for power of two
template<typename T>
inline constexpr bool isPower2(T n)
{
	static_assert(std::numeric_limits<T>::is_integer);
	return !((n - 1) & n);
}


//Only return the trailing digits of d
inline constexpr double trailing(double d)
{
	return d - (uint64_t)d;
}


//Linear Interpolation between d1 and d2 with the factor fac
template<typename T>
inline constexpr T lerp(T d1, T d2, double fac)
{
	return fac * d2 + (1.0 - fac) * d1;
}