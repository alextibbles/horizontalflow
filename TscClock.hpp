#if !defined(ELUCIDATE__UTIL__WRAP_HPP)
#define ELUCIDATE__UTIL__WRAP_HPP

#include <chrono>

namespace Elucidate::Util {

/** @brief wrap the rdtsc command as a chrono clock */
template< unsigned ClockSpeedHz, bool Safe >
struct TscClock {
	using rep = unsigned long long;
	using period = std::ratio< 1, ClockSpeedHz >;
	using duration = std::chrono::duration< rep, period >;
	using time_point = std::chrono::time_point< TscClock >;
	//static const bool is_steady = true; // really?
	static time_point now() noexcept {
		unsigned lo, hi;
		if (Safe) {
			asm volatile("rdtscp" : "=a" (lo), "=d" (hi));
		} else {
			asm volatile("rdtsc" : "=a" (lo), "=d" (hi));
		}
		return time_point(duration((static_cast< rep >(hi) << 32) | lo));
	}
};

}
#endif

