#if !defined(ELUCIDATE__SYS__SIGNAL_HPP)
#define ELUCIDATE__SYS__SIGNAL_HPP

#include "Stoppable.hpp"

#include <thread>
#include <atomic>
#include <signal.h>

namespace Elucidate::Sys {

	/**
	 * polls for signals, terminating the process on fatal.
	 * returns after timeout of 10ms.
	 */
	void processSignal();

	/**
	 * \brief protects the calling thread and children from async-unsafety
	 * creates a dedicated signal handling thread.
	 * signal handling thread is left to scheduler (no affinity).
	 * masks all signals for caller (inherited by children).
	 * on destruction reverses all changes for current thread.
	 * any child threads created within lifetime of SignalHandler
	 * should be destroyed before it.
	 */
	class SignalHandler {
	public:
		SignalHandler();
		~SignalHandler();
	private:
		std::atomic< int > shutdownSignal_;
		Util::Stoppable stopper_;
                std::thread signalThread_;
		sigset_t oldSet_;
	};

}

#endif

