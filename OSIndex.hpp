#if !defined(ELUCIDATE__SYS__OSINDEX_HPP)
#define ELUCIDATE__SYS__OSINDEX_HPP


namespace Elucidate::Sys {

// \brief representation of hardware location for OS
class OSIndex final {
public:
explicit OSIndex(unsigned osIndex) noexcept
:	osIndex_(osIndex)
{}
unsigned forOS() const noexcept {
	return osIndex_;
}

private:
unsigned osIndex_;
};
}
#endif

