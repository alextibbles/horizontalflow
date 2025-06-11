#if !defined(ELUCIDATE__UTIL__ERRORCODE_HPP)
#define ELUCIDATE__UTIL__ERRORCODE_HPP

#include "TracedTerminate.hpp"
#include "Trace.hpp"

#include <optional>

namespace Elucidate::Util {

/** @brief check a C-style error code, terminating on error */
template< typename T, typename U >
void checkReturnAndThrow(T const& goodValue, U const& actual, const char* error,
	std::optional< T > const& warnValue = std::optional< T >()) {
	if (goodValue != actual) {
		if (warnValue && (warnValue.value() == actual)) {
			LOG_WARN << "received " << warnValue.value() << " for " 
				<< error;
		} else {
			LOG_ERROR << actual;
			TRACED_TERMINATE(error);
		}
	}
}


/** @brief check a C-style error code, terminating on error */
template< typename T, typename U >
void checkForBadReturn(T const& badValue, U const& actual, const char* error) {
	if (badValue == actual) {
		LOG_ERROR << actual;
		TRACED_TERMINATE(error);
	}
}

}
#endif
