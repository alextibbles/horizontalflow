#if !defined(ELUCIDATE__DIS__ARITHMETIC_HPP)
#define ELUCIDATE__DIS__ARITHMETIC_HPP

#include <limits>

namespace Elucidate::Dis {

template< typename T >
consteval bool isPowerOf2(T t) noexcept {
	static_assert(std::is_unsigned< T >::value, 
		"bitwise requires unsigned");
	return t == (t & (~t + 1));
}

template< typename T >
consteval int countBits(T t) noexcept {
	int count = 0;
	while (t >>= 1) {
		++count;
	}
	return count;
}

template< int Size, typename IndexType >
IndexType bottomBound(IndexType ask) noexcept {
	if constexpr (std::is_signed< IndexType >::value) {
		return ask - Size;
	} else {
		if (ask <= Size) {
			return IndexType(0);
		} else {
			return ask - Size;
		}
	}
}

}
#endif

