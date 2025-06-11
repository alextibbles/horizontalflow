#if !defined(ELUCIDATE__UTIL__STOPPABLE_HPP)
#define ELUCIDATE__UTIL__STOPPABLE_HPP

#include <atomic>

namespace Elucidate::Util {

/** @brief run something until told otherwise.
 */
class Stoppable {
public:
	explicit Stoppable(std::atomic< int > const& stop)
	:	stop_(stop)
	{}

	template< typename F, typename ... Args >
	void operator()(F f, Args && ... args) const {
		while (!stop_.load(std::memory_order::relaxed)) {
			f(args...);
		}
	}

private:
	std::atomic< int > const& stop_;
};

}
#endif

