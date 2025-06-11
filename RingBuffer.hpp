#if !defined(ELUCIDATE__DIS__RINGBUFFER_HPP)
#define ELUCIDATE__DIS__RINGBUFFER_HPP

#include <array>
#include <atomic>

#include "Arithmetic.hpp"
#include "PaddedStore.hpp"
#include "Shuffle.hpp"

namespace Elucidate::Dis {

/** @brief array that exposes itself as an un-ending ring. user need not computes bounds
 */
template< typename ValueType, unsigned Size, unsigned DataPadSize, unsigned short ShuffleBits = 0 >
class RingBuffer final {
public:
	// @brief initialize, touching all pages preventing page faults later
	RingBuffer() noexcept;
	
	// @brief read-write element access
	// @param i index mod Size
	template< typename IndexType >
	std::atomic< ValueType >& operator[](IndexType i) noexcept;
	
	// @brief read-only element access
	// @param i index mod Size
	template< typename IndexType >
	std::atomic< ValueType > const& operator[](IndexType i) const noexcept;

private:
	// @brief return in-bounds index, unsigned as required by std::array
	template< typename IndexType >
	static constexpr auto getIndex(IndexType i) noexcept;
	
	RingBuffer(RingBuffer const&) = delete;
	RingBuffer(RingBuffer&&) = delete;
	RingBuffer& operator=(RingBuffer const&) = delete;
	RingBuffer& operator=(RingBuffer&&) = delete;

	// contiguous storage for cache-friendliness
	// cache-line-aligned items to avoid false-sharing
	using Array = std::array< PaddedStore< std::atomic< ValueType >, DataPadSize >, Size >;
	Array arr_;
	
	// want to use bitmask for wrapping, which only works when the size is a power of two
	static_assert(isPowerOf2(Size), "Size must be a power of 2");
	// eg. Size==8 => mask_==111b
	static constexpr std::size_t mask_ = Size - 1;
	
	static_assert(((1 << (ShuffleBits * 2)) - 1) <= mask_, "shuffle bits must take up half or less of the space");
};

template< typename ValueType, unsigned Size, unsigned DataPadSize, unsigned short ShuffleBits >
RingBuffer< ValueType, Size, DataPadSize, ShuffleBits >::RingBuffer() noexcept
:	arr_()
{}

template< typename ValueType, unsigned Size, unsigned DataPadSize, unsigned short ShuffleBits >
template< typename IndexType >
constexpr auto RingBuffer< ValueType, Size, DataPadSize, ShuffleBits >::getIndex(IndexType i) noexcept {
	// masking forces index in-bounds
	if constexpr (ShuffleBits != 0) {
		// shuffle also masks
		return shuffle< ShuffleBits, mask_ >(std::size_t(i));
	} else {
		return typename Array::size_type(i) & mask_;
	}
}

template< typename ValueType, unsigned Size, unsigned DataPadSize, unsigned short ShuffleBits >
template< typename IndexType >
std::atomic< ValueType >& RingBuffer< ValueType, Size, DataPadSize, ShuffleBits >::operator[](IndexType i) noexcept {
	return arr_[getIndex(i)].v;
}

template< typename ValueType, unsigned Size, unsigned DataPadSize, unsigned short ShuffleBits >
template< typename IndexType >
std::atomic< ValueType > const& RingBuffer< ValueType, Size, DataPadSize, ShuffleBits >::operator[](IndexType i) const noexcept {
	return arr_[getIndex(i)].v;
}

}
#endif

