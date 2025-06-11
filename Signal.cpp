#include "Signal.hpp"
#include "ErrorCode.hpp"

#include <signal.h>

using namespace Elucidate::Sys;
using namespace Elucidate::Util;

namespace {
void maskAllSignals(sigset_t& old) {
	sigset_t target;
	checkReturnAndThrow(0, sigemptyset(&target), "sigemptyset");
	checkReturnAndThrow(0,
		pthread_sigmask(SIG_SETMASK, &target, &old), "pthread_sigmask");
}
void restoreSignals(sigset_t& old) {
	checkReturnAndThrow(0,
		pthread_sigmask(SIG_SETMASK, &old, 0), "pthread_sigmask");
}
}

void Elucidate::Sys::processSignal() {
	sigset_t set;
	Util::checkReturnAndThrow(0, sigfillset(&set), "sigfillset");
	siginfo_t info;
	timespec timeout{0/*s*/, 10000000/*ns*/};
	auto s = sigtimedwait(&set, &info, &timeout);
	switch (s) {
	case -1: { // errors
		switch (errno) {
		case EAGAIN:
		default:
			break;
		case EINVAL:
			TRACED_TERMINATE("invalid timeout");
		}
		} break;

	case SIGINT:
	case SIGTERM:
		LOG_FATAL << "SIGINT/TERM";
		exit(-1);
		break;

	case SIGQUIT:
		TRACED_TERMINATE("SIGQUIT");
		break;

	case SIGCHLD:
	case SIGALRM:
	case SIGVTALRM:
	case SIGPROF:
	case SIGCONT:
	case SIGHUP:
	case SIGPIPE:
	case SIGPOLL:
	case SIGSTOP:
	case SIGTSTP:
	default:
		LOG_INFO << "continuing after signal " << s;
		break;
	}
}

SignalHandler::SignalHandler()
:	shutdownSignal_{0}
,	stopper_(shutdownSignal_)
,	signalThread_(stopper_, Sys::processSignal)
,	oldSet_{}
{
	// now there is a thread to deal with signals, mask them all
	// child threads inherit
	maskAllSignals(oldSet_);
}

SignalHandler::~SignalHandler()
{
	// remove the mask before stopping the thread
	restoreSignals(oldSet_);
	shutdownSignal_.store(1, std::memory_order_relaxed);
	signalThread_.join();
}

