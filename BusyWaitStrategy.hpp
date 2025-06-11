#if !defined(ELUCIDATE__DIS__BUSYWAITSTRATEGY_HPP)
#define ELUCIDATE__DIS__BUSYWAITSTRATEGY_HPP

#include "ARCount.hpp"
#include "Arithmetic.hpp"

namespace Elucidate::Dis {

/** @brief implements the waiting strategy that spins a core.
 * collaborates with RingBuffer.
 */
struct BusyWaitStrategy final {
	template< typename IndexType, unsigned PadSize >
	static IndexType waitFor(ARCount< IndexType, PadSize > const& cursor, IndexType ask) noexcept;
	template< typename IndexType, int Size, unsigned PadSize >
	static void waitForWrap(ARCount< IndexType, PadSize > const& cursor, IndexType ask) noexcept;
};

template< typename IndexType, unsigned PadSize >
inline IndexType BusyWaitStrategy::waitFor(ARCount< IndexType, PadSize > const& cursor, 
	IndexType ask) noexcept {
	IndexType ret;
	while ((ret = cursor.get()) < ask)
	{} // busy wait
	return ret;
}

template< typename IndexType, int Size, unsigned PadSize >
void BusyWaitStrategy::waitForWrap(ARCount< IndexType, PadSize > const& cursor, IndexType ask) noexcept {
	ask = bottomBound< Size >(ask);
	while (cursor.get() < ask)
	{} // busy wait
}

}
#endif

