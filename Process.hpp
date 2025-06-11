#if !defined(ELUCIDATE__SYS__PROCESS_HPP)
#define ELUCIDATE__SYS__PROCESS_HPP

#include <sys/types.h>

namespace Elucidate::Sys {

pid_t getProcessId();

void coreLock();

}

#endif

