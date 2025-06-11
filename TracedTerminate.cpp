#include "TracedTerminate.hpp"
#include "Trace.hpp"
#include "ToString.hpp"

#include <sstream>

void Elucidate::Util::traceAndTerminate(std::string const& s, const std::source_location& loc, 
		std::vector< std::string >&& trace) {	
	std::stringstream sstream;
	sstream << s << " from file " << loc.file_name() << " at line " << loc.line()
			<< " with trace " << printEach(trace, ", ");
	LOG_FATAL << sstream.str();
	std::terminate();
}

