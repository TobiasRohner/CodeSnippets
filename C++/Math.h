#pragma once


//A compile-time abs function
template<typename T>
constexpr T cabs(T n)
{
	return n < 0 ? -n : n;
}