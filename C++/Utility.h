#ifndef UTILITY_H
#define UTILITY_H


//Returns the size od the given array as long as its not just a pointer
template<typename T, size_t N>
constexpr size_t arraysize(T (&)[N]) { return N; }


#endif //UTILITY_H