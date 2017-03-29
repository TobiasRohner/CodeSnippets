#pragma once

#include <vector>
#include <cstdint>
#include <limits>


//Run Length Encoding (RLE)
namespace RLE
{

	//Performs Run Length Encoding (RLE-Compression) on a container of integer scalars
	//Uses the MSB as an indicator for repetitions:
	//	1: The current value is a counter telling how many times the next element should be repeated
	//	0: The current value can be copied as is in the decoded array
	template<typename Iterator>
	std::vector<typename Iterator::value_type> compress(Iterator begin, Iterator end)
	{
		static_assert(std::numeric_limits<Iterator::value_type>::is_integer, "Can only compress Integer Datatypes!");

		//Some useful declarations
		using stype = Iterator::value_type;
		using utype = std::make_unsigned<stype>::type;	//Unsigned version of the iterator value type (Used for counting reoccurances)
		const utype MSB = 1 << (std::numeric_limits<utype>::digits - 1);
		const utype MAX_COUNT = std::numeric_limits<utype>::max() >> 1;

		std::vector<stype> encoded;
		utype count = 1;
		stype last_element = *begin;

		//Compress all but the last element
		for (Iterator it = begin + 1 ; it != end ; ++it) {
			//If the element has to be stored
			if (*it != last_element || count == MAX_COUNT) {
				//Special case for only one occurance
				if (count == 1 && !(last_element & MSB))
					encoded.push_back(last_element);
				else {
					encoded.push_back(MSB | count);
					encoded.push_back(last_element);
				}
				last_element = *it;
				count = 0;
			}
			++count;
		}
		//Add the last element to the compressed vector
		if (count == 1 && !(last_element & MSB))
			encoded.push_back(last_element);
		else {
			encoded.push_back(MSB | count);
			encoded.push_back(last_element);
		}

		return encoded;
	}


	//Decompresses data which was compressed with RLE
	template<typename Iterator>
	std::vector<typename Iterator::value_type> decompress(Iterator begin, Iterator end)
	{
		static_assert(std::numeric_limits<Iterator::value_type>::is_integer, "Can only decompress Integer Datatypes!");

		//Some useful declarations
		using stype = Iterator::value_type;
		using utype = std::make_unsigned<stype>::type;	//Unsigned version of the iterator value type (Used for interpreting count variables)
		const utype MSB = 1 << (std::numeric_limits<utype>::digits - 1);
		const utype MAX_COUNT = std::numeric_limits<utype>::max() >> 1;

		std::vector<stype> encoded;

		Iterator it = begin;
		while (it != end) {
			if (*it & MSB) {		//*it is a counter indicating how many times the next element should be repeated
				utype count = *it - MSB;
				++it;
				encoded.reserve(encoded.size() + count);
				for (size_t i = 0; i < count; ++i)
					encoded.push_back(*it);
			}
			else	//The element can be inserted in the decompressed vector as is
				encoded.push_back(*it);
			++it;
		}

		return encoded;
	}

}