#if !defined(ELUCIDATE__DIS__SEQUENCER_HPP)
#define ELUCIDATE__DIS__SEQUENCER_HPP

#include "ARCount.hpp"
#include "PaddedStore.hpp"

#include <cassert>

namespace Elucidate::Dis {

/** @brief controls consumption and publication into a RingBuffer.
 * collaborates with RingBuffer. 2-phase publication: s.next(), pass barrier, s.publish()
 * only supports single dependencies, so 1-1 relationships only.
 */
template< typename IndexType, unsigned Size, typename WaitStrategy, unsigned PadSize >
struct Sequencer {
public:
	Sequencer(ARCount< IndexType, PadSize >& cursor, ARCount< IndexType, PadSize >& dependent, PaddedStore< IndexType, PadSize >& pending) noexcept;
	IndexType next() noexcept;
	void publish(IndexType next) noexcept;

	ARCount< IndexType, PadSize > &cursor_, &dep_;
	PaddedStore< IndexType, PadSize >& pending_;
};

template< typename IndexType, unsigned Size, typename WaitStrategy, unsigned PadSize >
Sequencer< IndexType, Size, WaitStrategy, PadSize >::Sequencer(ARCount< IndexType, PadSize >& cursor, ARCount< IndexType, PadSize >& dependent, PaddedStore< IndexType, PadSize >& pending) noexcept
:	cursor_(cursor)
,	dep_(dependent)
,	pending_(pending)
{}

template< typename IndexType, unsigned Size, typename WaitStrategy, unsigned PadSize >
IndexType Sequencer< IndexType, Size, WaitStrategy, PadSize >::next() noexcept {
	++pending_.v;
	WaitStrategy::template waitForWrap< IndexType, Size >(dep_, pending_.v);
	return pending_.v;
}

template< typename IndexType, unsigned Size, typename WaitStrategy, unsigned PadSize >
void Sequencer< IndexType, Size, WaitStrategy, PadSize >::publish(IndexType next) noexcept {
	assert(next == pending_.v);
	cursor_.set(next);
}

}
#endif

