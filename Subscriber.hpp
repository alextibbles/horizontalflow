#if !defined(ELUCIDATE__DIS__SUBSCRIBER_HPP)
#define ELUCIDATE__DIS__SUBSCRIBER_HPP

#include <cassert>

namespace Elucidate::Dis {

/** @brief consume in the minimal Disruptor pattern.
 * implements batched execution of Worker::operator() with single cursor update.
 */
template< typename WaitStrategy, typename RB, typename Counter, typename Worker >
void subscribe(RB const& rb, Counter& s, Counter const& dep, Worker w) noexcept {
	auto toConsume = s.get();
	++toConsume;
	const auto avail = WaitStrategy::waitFor(dep, toConsume);
	assert(avail >= toConsume);
	while (toConsume <= avail) {
		w(rb[toConsume].load(std::memory_order::acquire));
		++toConsume;
	}
	s.set(avail);
}

}
#endif

