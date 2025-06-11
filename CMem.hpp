#if !defined(ELUCIDATE__UTIL__CMEM_HPP)
#define ELUCIDATE__UTIL__CMEM_HPP

#include <cstdlib>

namespace Elucidate::Util {

/** @brief hold C-assigned memory */
template< typename T >
class CMem final {
public:
	CMem()
	:	t_()
	{}

	explicit CMem(T t)
	:	t_(t)
	{}

	~CMem() {
		std::free(t_);
	}

	const T get() const {
		return t_;
	}

	CMem(CMem< T > &&) = delete;
	CMem(CMem< T > const&) = delete;
	CMem& operator=(CMem< T > const&) = delete;
private:
	T t_;
};

}
#endif

