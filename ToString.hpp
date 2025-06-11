#if !defined(ELUCIDATE__UTIL__TOSTRING_HPP)
#define ELUCIDATE__UTIL__TOSTRING_HPP

#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <variant>
#include <iomanip>
#include <climits>
#include <boost/variant/recursive_wrapper.hpp>

namespace Elucidate::Util {
template< typename T >
std::string toString(T const& t) {
	std::stringstream s;
	s << t;
	return s.str();
}

template< typename T >
std::string toString(boost::recursive_wrapper<T> const& t) {
	return toString(t.get());
}

template< typename... Members >
std::string toString(std::variant< Members... > const& v) {
	return std::visit([](auto& t) { return toString(t); }, v);
}

template< typename T, typename U >
std::string toString(std::pair< T, U > const& p) {
	std::stringstream s;
	s << p.first;
	s << ":";
	s << p.second;
	return s.str();
}

template< typename T >
std::string dumpHex(T t) {
	constexpr std::size_t NUMBITS_PERHEX = 4;
	std::stringstream out;
	// prefer hardcoded 0x to std::showbase to prevent 0 showing without 0x
	out << std::hex << std::internal << std::showbase
		<< std::setfill('0') << std::setw(sizeof(T) * CHAR_BIT / NUMBITS_PERHEX)
		<< t;
	return out.str();
}

template< typename T >
std::string printEach(T const& t, const char *separator) {
	std::stringstream s;
	bool first = true;
	for (auto const& x : t) {
		if (!first) {
			s << separator;
		}
		s << x;
		first = false;
	}
	return s.str();
}

template<>
std::string printEach< std::vector< char* > >(
	std::vector< char* > const& t, const char *separator);

template<>
std::string printEach< std::pair< int, char** > >(
	std::pair< int, char** > const& t, const char *separator);

}
#endif
