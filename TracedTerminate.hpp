#if !defined(ELUCIDATE__UTIL__TRACEDTERMINATE_HPP)
#define ELUCIDATE__UTIL__TRACEDTERMINATE_HPP

#define TRACED_TERMINATE(msg) ::Elucidate::Util::traceAndTerminate(msg, std::source_location::current(), ::Elucidate::Util::backTrace())

#include <string>
#include <vector>
#include <source_location>

#include "BackTrace.hpp"

namespace Elucidate::Util {
void traceAndTerminate(std::string const& s, const std::source_location& loc, std::vector< std::string >&& trace);
}
#endif

