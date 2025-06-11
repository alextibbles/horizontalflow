#include "Process.hpp"
#include "ErrorCode.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <cstdlib>

pid_t Elucidate::Sys::getProcessId() {
	return getpid();
}

void Elucidate::Sys::coreLock() {
	Util::checkReturnAndThrow(0, mlockall(MCL_FUTURE | MCL_CURRENT),
		"mlock");
}

