#if !defined(ELUCIDATE__SYS__SLEEP_HPP)
#define ELUCIDATE__SYS__SLEEP_HPP

#include <time.h>

namespace Elucidate::Sys {

// \brief wrap nanosleep system call and compensate for signal interruptions
void sleep(long nanos) {
  timespec request{0, nanos}, remaining;
  while (0 != nanosleep(&request, &remaining)) {
    request.tv_nsec = remaining.tv_nsec;
  }
}

}
#endif

