#if !defined(ELUCIDATE__SYS__THREAD_HPP)
#define ELUCIDATE__SYS__THREAD_HPP

#include <optional>
#include <thread>
#include <sys/types.h>

namespace Elucidate::Sys {

class OSIndex;


/** @brief set CPU affinity of this thread to a single PU */
void setMyCPURange(OSIndex id);

/** @brief set CPU affinity of thread to a single PU */
void setCPURange(std::thread& t, OSIndex id, 
	std::optional< int > const& warnValue 
		= std::optional< int >());

/** @brief grab the POSIX thread id */
pthread_t getPThreadId();

/** @brief grab the linux thread id */
long getThreadId();

/** @brief set current thread to top priority */
void setMyHighPriority();

/** @brief set thread to top priority */
void setHighPriority(std::thread& t, 
        std::optional< int > const& warnValue 
                = std::optional< int >());

}
#endif

