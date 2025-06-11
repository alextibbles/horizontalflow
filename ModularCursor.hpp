#if !defined(ELUCIDATE__DIS__MODULARCURSOR_HPP)
#define ELUCIDATE__DIS__MODULARCURSOR_HPP

#include <compare>
#include <limits>
#include <cstdlib>
#include <cmath>
#include <cassert>

namespace Elucidate::Dis {

/** @brief index that wraps round
 * suitable for use as a cursor for a ring buffer
 * @param Format underlying format to contain the index
 * @param Distance maximum distance permitted between two cursors
 */
template< typename Format, int Distance >
class ModularCursor final {
public:
	ModularCursor() noexcept;
	explicit ModularCursor(Format f) noexcept;
	
	std::strong_ordering operator<=>(ModularCursor const& rhs) const noexcept;
	bool operator==(ModularCursor const&) const = default;
	
	ModularCursor& operator++() noexcept;
	ModularCursor& operator&=(ModularCursor const& rhs) noexcept;
	ModularCursor operator+(ModularCursor const& rhs) const noexcept;
	
	explicit operator std::size_t() const noexcept;

	static_assert(Distance < std::numeric_limits< Format >::max());
	
	template< int Size, typename F, int D >
	friend auto bottomBound(ModularCursor< F, D > const& ask) noexcept;
private:
	Format v_;
};

template< typename Format, int Distance >
inline ModularCursor< Format, Distance >::ModularCursor() noexcept
:	v_()
{}

template< typename Format, int Distance >
inline ModularCursor< Format, Distance >::ModularCursor(Format f) noexcept
:	v_(f)
{}

namespace detail {
template< typename >
struct BigEnoughForDifference;

template<>
struct BigEnoughForDifference< int >
{
	static_assert(sizeof(long) > sizeof(int));
	using type = long;
};
template<>
struct BigEnoughForDifference< short >
{
	static_assert(sizeof(int) > sizeof(short));
	using type = int;
};

template< typename T >
auto safeSubtract(T x, T y) {
	assert(x >= y);
	typename detail::BigEnoughForDifference< T >::type diff = x;
	diff -= y;
	return diff;
}
}

template< typename Format, int Distance >
inline std::strong_ordering ModularCursor< Format, Distance >::operator<=>(ModularCursor const& rhs) const noexcept {
	auto c = (v_ <=> rhs.v_);
	if (c == std::strong_ordering::equal) {
		return std::strong_ordering::equal;
	} else if (c == std::strong_ordering::greater) {
		// ie v_ > rhs.v_, however v_ - rhs.v_ may be greater than Format max
		if (detail::safeSubtract(v_, rhs.v_) < Distance) {
			return std::strong_ordering::greater;
		} else {
			// wrap, so lhs > rhs. check that it is within bounds
			assert(detail::safeSubtract(std::numeric_limits< Format >::max(), v_) + detail::safeSubtract(rhs.v_, std::numeric_limits< Format >::min()) < Distance);
			return std::strong_ordering::less;
		}
	} else {
		// ie v_ < rhs.v_, however rhs.v_ - v_ may be greater than Format max
		if (detail::safeSubtract(rhs.v_, v_) > Distance) {
			// wrap, so lhs > rhs. check that it is within bounds
			assert(detail::safeSubtract(std::numeric_limits< Format >::max(), rhs.v_) + detail::safeSubtract(v_, std::numeric_limits< Format >::min()) < Distance);
			return std::strong_ordering::greater;
		} else {
			// "normal" case, just lhs < rhs
			return std::strong_ordering::less;
		}
	}
}


template< typename Format, int Distance >
inline ModularCursor< Format, Distance >& ModularCursor< Format, Distance >::operator++() noexcept {
	++v_;
	return *this;
}

template< typename Format, int Distance >
inline ModularCursor< Format, Distance >::operator std::size_t() const noexcept {
	return static_cast< std::size_t >(v_);
}

template< typename Format, int Distance >
inline ModularCursor< Format, Distance > ModularCursor< Format, Distance >::operator+(ModularCursor const& rhs) const noexcept {
	return ModularCursor< Format, Distance >(v_ + rhs.v_);
}


template< typename Format, int Distance >
inline ModularCursor< Format, Distance >& ModularCursor< Format, Distance >::operator&=(ModularCursor const& rhs) noexcept {
	v_ &= rhs.v_;
	return *this;
}

// specialize
template< int Size, typename Format, int Distance >
auto bottomBound(ModularCursor< Format, Distance > const& ask) noexcept {
	return ModularCursor< Format, Distance >(bottomBound< Size, Format >(ask.v_));
}

}
#endif

