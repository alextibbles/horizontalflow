#include "Thread.hpp"
#include "ErrorCode.hpp"
#include "OSIndex.hpp"

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>

using namespace Elucidate::Sys;
using namespace Elucidate::Util;

namespace {
/** @brief set CPU affinity data to a single PU */
void setCPUSet(cpu_set_t& cpus, OSIndex pu) {
	CPU_ZERO(&cpus);
	CPU_SET(pu.forOS(), &cpus);
}

void setMyCPURangeImpl(pthread_t tid, OSIndex id, 
        std::optional< int > const& warnValue) {
	cpu_set_t cpus;
	setCPUSet(cpus, id);
	checkReturnAndThrow(0, pthread_setaffinity_np(tid, 
		sizeof(cpu_set_t), &cpus), "pthread_setaffinity_np", warnValue);
}}

void Elucidate::Sys::setCPURange(std::thread& t, OSIndex id, 
        std::optional< int > const& warnValue) {
	setMyCPURangeImpl(t.native_handle(), id, warnValue);
}

void Elucidate::Sys::setMyCPURange(OSIndex id) {
	setMyCPURangeImpl(pthread_self(), id, std::optional< int >());
}

pthread_t Elucidate::Sys::getPThreadId() {
	return pthread_self();
}

long Elucidate::Sys::getThreadId() {
	return syscall(SYS_gettid);
}

namespace {
void setHighPriorityImpl(pthread_t tid,
        std::optional< int > const& warnValue) {
	sched_param param;
	param.sched_priority = sched_get_priority_max(SCHED_FIFO);
	checkReturnAndThrow(0, pthread_setschedparam(tid, 
		SCHED_FIFO, &param), "pthread_setschedparam", warnValue);
}}

void Elucidate::Sys::setHighPriority(std::thread& t,
        std::optional< int > const& warnValue) {
	setHighPriorityImpl(t.native_handle(), warnValue);
}

void Elucidate::Sys::setMyHighPriority() {
	setHighPriorityImpl(pthread_self(), std::optional< int >());
}

