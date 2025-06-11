#if !defined(ELUCIDATE__UTIL__BACKTRACE_HPP)
#define ELUCIDATE__UTIL__BACKTRACE_HPP

#include <vector>
#include <string>

namespace Elucidate::Util {

/** @brief grab the backtrace of symbols */
std::vector< std::string > backTrace();

}
#endif
