#if !defined(ELUCIDATE__DIS__ARCOUNT_HPP)
#define ELUCIDATE__DIS__ARCOUNT_HPP

#include "PaddedStore.hpp"

#include <atomic>

namespace Elucidate::Dis {

/** @brief acquire/release atomic counter
 */
template< typename CounterType, unsigned PadSize >
class ARCount final {
public:
	ARCount() noexcept;
	CounterType get() const noexcept;
	void set(CounterType v) noexcept;

private:
	PaddedStore< std::atomic< CounterType >, PadSize > sequence_;
};

template< typename CounterType, unsigned PadSize >
inline ARCount< CounterType, PadSize >::ARCount()  noexcept
:	sequence_()
{}

template< typename CounterType, unsigned PadSize >
inline CounterType ARCount< CounterType, PadSize >::get() const noexcept {
	return sequence_.v.load(std::memory_order::acquire);
}

template< typename CounterType, unsigned PadSize >
inline void ARCount< CounterType, PadSize >::set(CounterType next) noexcept {
	sequence_.v.store(next, std::memory_order::release);
}

}
#endif

