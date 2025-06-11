#if !defined(ELUCIDATE__DIS__SHUFFLE_HPP)
#define ELUCIDATE__DIS__SHUFFLE_HPP

namespace Elucidate::Dis {

template< unsigned short ShuffleBits >
auto shuffleMix(std::size_t i, std::size_t mix) noexcept {
	return i ^ mix ^ (mix << ShuffleBits);
}

template< unsigned short ShuffleBits, std::size_t Mask >
auto shuffle(std::size_t i) noexcept {
	return shuffleMix< ShuffleBits >(i, (i ^ (i >> ShuffleBits)) & ((1u << ShuffleBits) - 1)) & Mask;
}

}
#endif

