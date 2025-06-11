#if !defined(ELUCIDATE__DIS__CONTROL_HPP)
#define ELUCIDATE__DIS__CONTROL_HPP

#include "ARCount.hpp"

namespace Elucidate::Dis {

// \brief helper structure with control data
template< typename CounterType, unsigned PadSize >
struct Control {
	ARCount< CounterType, PadSize > pub, sub;
	PaddedStore< CounterType, PadSize > pending;
	
	Control() noexcept
	:	pub()
	,	sub()
	,	pending(pub.get())
	{}
};

}
#endif

