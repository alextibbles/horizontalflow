#if !defined(ELUCIDATE__DIS__PUBLISHER_HPP)
#define ELUCIDATE__DIS__PUBLISHER_HPP

namespace Elucidate::Dis {

/** @brief publish in the minimal Disruptor pattern.
 * implements transactional around advice for sending Data.
 */
template< typename RB, typename Sequencer, typename Data >
void publish(RB& rb, Sequencer& s, Data const& data) noexcept {
	auto next = s.next();
	rb[next].store(data, std::memory_order::release);
	s.publish(next);
}

}
#endif

