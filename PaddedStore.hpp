#if !defined(ELUCIDATE__DIS__PADDEDSTORE_HPP)
#define ELUCIDATE__DIS__PADDEDSTORE_HPP

#include "Arithmetic.hpp"

namespace Elucidate::Dis {

/** @brief padded object to cacheline.
 */
template< typename ValueType, std::size_t PadSize >
struct PaddedStore final {
	PaddedStore() noexcept
	:	v()
	{}
	
	template< typename ... Args >
	explicit PaddedStore(Args const& ... args) noexcept
	:	v(args...)
	{}

	static_assert(sizeof(ValueType) <= PadSize, "unsupported padding of large object");
	alignas(PadSize) ValueType v;

};

}
#endif

