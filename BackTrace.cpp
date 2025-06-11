#include "BackTrace.hpp"
#include "CMem.hpp"

#include <cstddef>
#include <execinfo.h>

// TODO replace with std::stacktrace_entry etc C++23

std::vector< std::string > Elucidate::Util::backTrace() {
	const std::size_t TRACE_SIZE_MAX(100);
	void *calls[TRACE_SIZE_MAX];
	const int size = backtrace(calls, TRACE_SIZE_MAX);
	CMem< char** > strings(backtrace_symbols(calls, size));
	std::vector< std::string > ret;
	for (int ii = 0; ii < size; ++ii) {
		ret.emplace_back(strings.get()[ii]);
	}
	return ret;
}
