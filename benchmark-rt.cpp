#include "TscClock.hpp"
#include "RingBuffer.hpp"
#include "ARCount.hpp"
#include "Sequencer.hpp"
#include "BusyWaitStrategy.hpp"
#include "Publisher.hpp"
#include "Subscriber.hpp"
#include "Sleep.hpp"
#include "Process.hpp"
#include "Thread.hpp"
#include "Signal.hpp"
#include "OSIndex.hpp"
#include "ModularCursor.hpp"
#include "BenchmarkITC.hpp"

#include <iostream>
#include <iomanip>
#include <tuple>
#include <thread>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <string>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#ifdef NDEBUG
constexpr auto HIPERF = true;
#else
constexpr auto HIPERF = false;
#endif

using namespace Elucidate::Util;
using namespace Elucidate::Dis;
using namespace Elucidate::UT;
namespace po = boost::program_options;
using namespace std::literals::string_literals;

namespace {
// @brief count of how many iterations to perform, each one costs ~30B of memory
constexpr int COUNT{10000000};
using IntegerWord = long;

template< typename CounterType, unsigned RBSize, unsigned DataPadSize, unsigned CounterPadSize, int ShuffleBits >
struct RBData {
  RBData()
  :	rb()
  ,	pub()
  ,	sub()
  ,	pending(pub.get())
  ,	seq(pub, sub, pending)
  {}

  RingBuffer< IntegerWord, RBSize, DataPadSize, ShuffleBits > rb;
  ARCount< CounterType, CounterPadSize > pub, sub;
  PaddedStore< CounterType, CounterPadSize > pending;
  Sequencer< CounterType, RBSize, BusyWaitStrategy, CounterPadSize > seq;
};

template< typename CounterType, unsigned RBSize, unsigned DataPadSize, unsigned CounterPadSize, int ShuffleBits >
struct RBProducer {
  explicit RBProducer(Parms< RBData< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits >, COUNT >& data)
  :	data_(data)
  {}
  Parms< RBData< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits >, COUNT >& data_;
  void operator()(IntegerWord val) {
    publish(data_.buffers.rb, data_.buffers.seq, val);
  }
};

template< typename ITCData, bool Intrusive >
struct DisruptorEventProcessor {
  // take count in as OUT parameter since we get called in blocks, so only EVP knows how many RXes there are
  explicit DisruptorEventProcessor(ITCData& parms)
  :	parms_(parms)
  {}

  void operator()(IntegerWord const& value) {
    parms_.responses[typename decltype(parms_.responses)::size_type(parms_.responseCounter)] = value;
    if constexpr (Intrusive) {
      parms_.rx[typename decltype(parms_.rx)::size_type(parms_.responseCounter)] = IntrusiveUnsafeClock::now();
    }
    ++parms_.responseCounter;
  }
  ITCData& parms_;
};

template< typename ITCData, bool Intrusive >
struct RBPartner {
static void work(ITCData &parms) {
  for (parms.responseCounter = 0; parms.responseCounter < COUNT;) {
    subscribe< BusyWaitStrategy >(parms.buffers.rb, parms.buffers.sub,
      parms.buffers.pub, DisruptorEventProcessor< ITCData, Intrusive >(parms));
  }
}
};

using Latencies = std::array< IntrusiveUnsafeClock::duration, COUNT >;
using Intervals = std::array< IntrusiveUnsafeClock::duration, COUNT - 1 >;

struct Id {
	auto operator()(IntegerWord ii) const {
		return ii;
	}
};

template< typename CounterType, unsigned RBSize, unsigned DataPadSize, unsigned CounterPadSize, unsigned short ShuffleBits, bool Intrusive, bool Pause, bool Metrics >
auto runRB() {
  using RBBenchmarkData = Parms< RBData< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits >, COUNT >;
  return benchmarkITC< RBBenchmarkData, COUNT, 
  	TestDriver< RBBenchmarkData, COUNT, 
  	  RBProducer< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits >, Intrusive, Pause >, 
  	RBPartner< RBBenchmarkData, Intrusive >, 
  	Intrusive, Id, Pause, Metrics, HIPERF >();
}

constexpr const char *OPTION_HELP("help");
constexpr const char *OPTION_METRICS("metrics");
constexpr const char *OPTION_TEST("test");
constexpr const char *OPTION_INTRUSIVE("intrusive");
constexpr const char *OPTION_PAUSE("pause");

constexpr const char *OPTION_TEST_DEFAULT = "all";
constexpr const char *OPTION_TEST_RB = "rb";
constexpr const char *OPTION_TEST_RBM = "rbM";
constexpr const char *OPTION_TEST_RBMS = "rbMs";
constexpr const char *OPTION_TEST_RBMSS = "rbMss";
constexpr const char *OPTION_TEST_RBMXS = "rbMxs";
constexpr const char *OPTION_TEST_RBL = "rbL";
constexpr const char *OPTION_TEST_RBLS = "rbLs";
constexpr const char *OPTION_TEST_RBLSS = "rbLss";
constexpr const char *OPTION_TEST_RBLNS = "rbLns";
constexpr const char *OPTION_TEST_RBLHS = "rbLhs";
constexpr const char *OPTION_TEST_RBLDS = "rbLds";
constexpr const char *OPTION_TEST_RBLCS = "rbLcs";
constexpr const char *OPTION_TEST_RBLIS = "rbLis";
constexpr const char *OPTION_TEST_RBLTS = "rbLts";
constexpr const char *OPTION_TEST_RBLXS = "rbLxs";
constexpr const char *OPTION_TEST_RBX = "rbX";
constexpr const char *OPTION_TEST_RBXS = "rbXs";
constexpr const char *OPTION_TEST_RBXSS = "rbXss";
constexpr const char *OPTION_TEST_RBXNS = "rbXns";
constexpr const char *OPTION_TEST_RBXHS = "rbXhs";
constexpr const char *OPTION_TEST_RBXDS = "rbXds";
constexpr const char *OPTION_TEST_RBXCS = "rbXcs";
constexpr const char *OPTION_TEST_RBXXS = "rbXxs";

enum class Tests {
RB,
RBM,
RBMS,
RBMSS,
RBMXS,
RBL,
RBLS,
RBLSS,
RBLNS,
RBLHS,
RBLDS,
RBLCS,
RBLIS,
RBLTS,
RBLXS,
RBX,
RBXS,
RBXSS,
RBXNS,
RBXHS,
RBXDS,
RBXCS,
RBXXS,
};

constexpr auto testName(Tests test) {
  switch (test) {
    case Tests::RB:
      return OPTION_TEST_RB;
    case Tests::RBM:
      return OPTION_TEST_RBM;
    case Tests::RBMS:
      return OPTION_TEST_RBMS;
    case Tests::RBMSS:
      return OPTION_TEST_RBMSS;
    case Tests::RBMXS:
      return OPTION_TEST_RBMXS;
    case Tests::RBL:
      return OPTION_TEST_RBL;
    case Tests::RBLS:
      return OPTION_TEST_RBLS;
    case Tests::RBLSS:
      return OPTION_TEST_RBLSS;
    case Tests::RBLNS:
      return OPTION_TEST_RBLNS;
    case Tests::RBLHS:
      return OPTION_TEST_RBLHS;
    case Tests::RBLDS:
      return OPTION_TEST_RBLDS;
    case Tests::RBLCS:
      return OPTION_TEST_RBLCS;
    case Tests::RBLIS:
      return OPTION_TEST_RBLIS;
    case Tests::RBLTS:
      return OPTION_TEST_RBLTS;
    case Tests::RBLXS:
      return OPTION_TEST_RBLXS;
    case Tests::RBX:
      return OPTION_TEST_RBX;
    case Tests::RBXS:
      return OPTION_TEST_RBXS;
    case Tests::RBXSS:
      return OPTION_TEST_RBXSS;
    case Tests::RBXNS:
      return OPTION_TEST_RBXNS;
    case Tests::RBXHS:
      return OPTION_TEST_RBXHS;
    case Tests::RBXDS:
      return OPTION_TEST_RBXDS;
    case Tests::RBXCS:
      return OPTION_TEST_RBXCS;
    case Tests::RBXXS:
      return OPTION_TEST_RBXXS;
  }
  assert(false);
}

using Functions = std::map< std::tuple< Tests, bool, bool, bool >, std::function< Result() > >;

template< Tests test, typename CounterType, unsigned RBSize, unsigned DataPadSize, unsigned CounterPadSize, unsigned short ShuffleBits >
void addRBTestSet(Functions &functions) {
  functions.insert(std::pair(std::tuple(test, false, false, false), 
  	std::function< Result() >(runRB< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits, false, false, false >)));
  functions.insert(std::pair(std::tuple(test, false, false, true), 
  	std::function< Result() >(runRB< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits, false, false, true >)));
  functions.insert(std::pair(std::tuple(test, true, false, false), 
  	std::function< Result() >(runRB< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits, true, false, false >)));
  functions.insert(std::pair(std::tuple(test, true, false, true), 
  	std::function< Result() >(runRB< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits, true, false, true >)));
  functions.insert(std::pair(std::tuple(test, true, true, false), 
  	std::function< Result() >(runRB< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits, true, true, false >)));
  functions.insert(std::pair(std::tuple(test, true, true, true), 
  	std::function< Result() >(runRB< CounterType, RBSize, DataPadSize, CounterPadSize, ShuffleBits, true, true, true >)));
}
}

int main(int argc, char **argv) {
  bool metrics = true, instrusive = false, pause = false;
  po::options_description desc("Allowed options");
  desc.add_options()
    (OPTION_HELP, "produce help message")
    (OPTION_METRICS, 
      po::value< bool >(&metrics)->default_value(true), "generate metrics")
    (OPTION_TEST,
      po::value< std::string >()->default_value(OPTION_TEST_DEFAULT),
      "test to run")
    (OPTION_INTRUSIVE, po::value< bool >(&instrusive)->default_value(false), "use intrusive timers")
    (OPTION_PAUSE, po::value< bool >(&pause)->default_value(false), "conduct a periodic latency test (implies instrusive=true)")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);
  
  if (vm.count(OPTION_HELP)) {
    std::cout << desc << '\n';
    return 0;
  }
  
  if (metrics) {
    std::cerr
      << "thrpt=average throughput measured externally,\n"
        "lat=latency between transmission and receipt,\n"
        "int=interval between one transmission and the next since blocking\n"
      << "\tthrpt"
      << "\tloss"
      << "\tmin lat"
      << "\tmin int"
      << "\tavg lat"
      << "\tavg int"
      << "\tmed lat"
      << "\tmed int"
      << "\tpk lat"
      << "\tpk int"
      << "\n";
  }
  
  std::vector< std::tuple< Tests, bool, bool, bool > > tests;
  if (OPTION_TEST_DEFAULT == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RB, false, false, metrics);
    tests.emplace_back(Tests::RB, true, false, metrics);
    tests.emplace_back(Tests::RB, true, true, metrics);
    tests.emplace_back(Tests::RBM, false, false, metrics);
    tests.emplace_back(Tests::RBM, true, false, metrics);
    tests.emplace_back(Tests::RBM, true, true, metrics);
    tests.emplace_back(Tests::RBMS, false, false, metrics);
    tests.emplace_back(Tests::RBMS, true, false, metrics);
    tests.emplace_back(Tests::RBMS, true, true, metrics);
    tests.emplace_back(Tests::RBMSS, false, false, metrics);
    tests.emplace_back(Tests::RBMSS, true, false, metrics);
    tests.emplace_back(Tests::RBMSS, true, true, metrics);
    tests.emplace_back(Tests::RBMXS, false, false, metrics);
    tests.emplace_back(Tests::RBMXS, true, false, metrics);
    tests.emplace_back(Tests::RBMXS, true, true, metrics);
    tests.emplace_back(Tests::RBL, false, false, metrics);
    tests.emplace_back(Tests::RBL, true, false, metrics);
    tests.emplace_back(Tests::RBL, true, true, metrics);
    tests.emplace_back(Tests::RBLS, false, false, metrics);
    tests.emplace_back(Tests::RBLS, true, false, metrics);
    tests.emplace_back(Tests::RBLS, true, true, metrics);
    tests.emplace_back(Tests::RBLSS, false, false, metrics);
    tests.emplace_back(Tests::RBLSS, true, false, metrics);
    tests.emplace_back(Tests::RBLSS, true, true, metrics);
    tests.emplace_back(Tests::RBLNS, false, false, metrics);
    tests.emplace_back(Tests::RBLNS, true, false, metrics);
    tests.emplace_back(Tests::RBLNS, true, true, metrics);
    tests.emplace_back(Tests::RBLHS, false, false, metrics);
    tests.emplace_back(Tests::RBLHS, true, false, metrics);
    tests.emplace_back(Tests::RBLHS, true, true, metrics);
    tests.emplace_back(Tests::RBLDS, false, false, metrics);
    tests.emplace_back(Tests::RBLDS, true, false, metrics);
    tests.emplace_back(Tests::RBLDS, true, true, metrics);
    tests.emplace_back(Tests::RBLCS, false, false, metrics);
    tests.emplace_back(Tests::RBLCS, true, false, metrics);
    tests.emplace_back(Tests::RBLCS, true, true, metrics);
    tests.emplace_back(Tests::RBLIS, false, false, metrics);
    tests.emplace_back(Tests::RBLIS, true, false, metrics);
    tests.emplace_back(Tests::RBLIS, true, true, metrics);
    tests.emplace_back(Tests::RBLTS, false, false, metrics);
    tests.emplace_back(Tests::RBLTS, true, false, metrics);
    tests.emplace_back(Tests::RBLTS, true, true, metrics);
    tests.emplace_back(Tests::RBLXS, false, false, metrics);
    tests.emplace_back(Tests::RBLXS, true, false, metrics);
    tests.emplace_back(Tests::RBLXS, true, true, metrics);
    tests.emplace_back(Tests::RBX, false, false, metrics);
    tests.emplace_back(Tests::RBX, true, false, metrics);
    tests.emplace_back(Tests::RBX, true, true, metrics);
    tests.emplace_back(Tests::RBXS, false, false, metrics);
    tests.emplace_back(Tests::RBXS, true, false, metrics);
    tests.emplace_back(Tests::RBXS, true, true, metrics);
    tests.emplace_back(Tests::RBXSS, false, false, metrics);
    tests.emplace_back(Tests::RBXSS, true, false, metrics);
    tests.emplace_back(Tests::RBXSS, true, true, metrics);
    tests.emplace_back(Tests::RBXNS, false, false, metrics);
    tests.emplace_back(Tests::RBXNS, true, false, metrics);
    tests.emplace_back(Tests::RBXNS, true, true, metrics);
    tests.emplace_back(Tests::RBXHS, false, false, metrics);
    tests.emplace_back(Tests::RBXHS, true, false, metrics);
    tests.emplace_back(Tests::RBXHS, true, true, metrics);
    tests.emplace_back(Tests::RBXDS, false, false, metrics);
    tests.emplace_back(Tests::RBXDS, true, false, metrics);
    tests.emplace_back(Tests::RBXDS, true, true, metrics);
    tests.emplace_back(Tests::RBXCS, false, false, metrics);
    tests.emplace_back(Tests::RBXCS, true, false, metrics);
    tests.emplace_back(Tests::RBXCS, true, true, metrics);
    tests.emplace_back(Tests::RBXXS, false, false, metrics);
    tests.emplace_back(Tests::RBXXS, true, false, metrics);
    tests.emplace_back(Tests::RBXXS, true, true, metrics);
  } else if (OPTION_TEST_RB == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RB, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBM == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBM, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBMS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBMS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBMSS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBMSS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBMXS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBMXS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBL == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBL, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLSS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLSS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLNS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLNS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLHS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLHS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLDS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLDS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLCS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLCS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLIS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLIS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLTS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLTS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBLXS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBLXS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBX == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBX, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXSS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXSS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXNS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXNS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXHS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXHS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXDS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXDS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXCS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXCS, instrusive, pause, metrics);
  } else if (OPTION_TEST_RBXXS == vm[OPTION_TEST].as< std::string >()) {
    tests.emplace_back(Tests::RBXXS, instrusive, pause, metrics);
  }
  
  std::map< std::tuple< Tests, bool, bool, bool >, std::function< Result() > > functions;

  // test a ring buffer with only 1 entry(!)
  addRBTestSet< Tests::RB, std::size_t, 1, 128, 128, 0 >(functions);

  // test a ring buffer with 64 entries so it allows batching
  addRBTestSet< Tests::RBM, std::size_t, 64, 128, 128, 0 >(functions);

  // test a ring buffer with 64 entries so it allows batching
  // small padding to increase amount in l1/l2
  addRBTestSet< Tests::RBMS, std::size_t, 64, 64, 64, 0 >(functions);

  // test a ring buffer with 64 entries so it allows batching
  // small padding to increase amount in l1/l2
  // use signed counters
  addRBTestSet< Tests::RBMSS, ssize_t, 64, 64, 64, 0 >(functions);

  // test a ring buffer with 64 entries so it allows batching
  // use shuffling to prevent sequential access by readers and writers contending, and don't pad data
  // use signed counters
  constexpr int SHUFFLE = countBits(64 / sizeof(IntegerWord));
  addRBTestSet< Tests::RBMXS, ssize_t, 64, sizeof(IntegerWord), 64, SHUFFLE >(functions);

  // test a ring buffer with 1024 entries so it allows queuing
  // the producer can run ahead of the consumer, increasing latency
  // the producer can run asynchronously, increasing throughput
  addRBTestSet< Tests::RBL, std::size_t, 1024, 128, 128, 0 >(functions);

  // test a ring buffer with 1024 entries so it allows queuing
  // smaller padding to better fit l1/l2
  addRBTestSet< Tests::RBLS, std::size_t, 1024, 64, 64, 0 >(functions);

  // test a ring buffer with 1024 entries so it allows queuing
  // smaller padding to better fit l1/l2
  // use signed counters
  addRBTestSet< Tests::RBLSS, ssize_t, 1024, 64, 64, 0 >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // no padding to see what constraint cacheline contention exerts
  // use signed counters
  addRBTestSet< Tests::RBLNS, ssize_t, 1024, sizeof(IntegerWord), sizeof(IntegerWord), 0 >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // only counter padding with bit shuffling to disperse data accesses
  // use signed counters
  addRBTestSet< Tests::RBLHS, ssize_t, 1024, sizeof(IntegerWord), 64, SHUFFLE >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // only data padding to see what constraint cacheline contention exerts
  // use signed counters
  addRBTestSet< Tests::RBLDS, ssize_t, 1024, 64, sizeof(IntegerWord), 0 >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // only counter padding to see what constraint cacheline contention exerts
  // use signed counters
  addRBTestSet< Tests::RBLCS, ssize_t, 1024, sizeof(IntegerWord), 64, 0 >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // only counter padding to see what constraint cacheline contention exerts
  // use signed counters, truncated into 16bits
  addRBTestSet< Tests::RBLIS, ModularCursor< short, 1024 >, 1024, sizeof(IntegerWord), 64, 0 >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // only counter padding to see what constraint cacheline contention exerts
  // use signed counters, truncated into 32bits
  addRBTestSet< Tests::RBLTS, ModularCursor< int, 1024 >, 1024, sizeof(IntegerWord), 64, 0 >(functions);
  
  // test a ring buffer with 1024 entries so it allows queuing
  // counter padding combined with data shuffling
  // use signed counters
  addRBTestSet< Tests::RBLXS, ssize_t, 1024, sizeof(IntegerWord), 64, SHUFFLE >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // the producer can run ahead of the consumer, increasing latency
  // the producer can run asynchronously, increasing throughput
  addRBTestSet< Tests::RBX, std::size_t, 64*1024, 128, 128, 0 >(functions);
  
  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // smaller padding to better fit l1/l2
  addRBTestSet< Tests::RBXS, std::size_t, 64*1024, 64, 64, 0 >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // smaller padding to better fit l1/l2
  // use signed counters
  addRBTestSet< Tests::RBXSS, ssize_t, 64*1024, 64, 64, 0 >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // no padding to assess contention
  // use signed counters
  addRBTestSet< Tests::RBXNS, ssize_t, 64*1024, sizeof(IntegerWord), sizeof(IntegerWord), 0 >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // no padding but shuffling
  // use signed counters
  addRBTestSet< Tests::RBXHS, ssize_t, 64*1024, sizeof(IntegerWord), sizeof(IntegerWord), SHUFFLE >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // data padding only to assess contention
  // use signed counters
  addRBTestSet< Tests::RBXDS, ssize_t, 64*1024, 64, sizeof(IntegerWord), 0 >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // counter padding only to assess contention
  // use signed counters
  addRBTestSet< Tests::RBXCS, ssize_t, 64*1024, sizeof(IntegerWord), 64, 0 >(functions);

  // test a huge ring buffer with 64*1024 entries so it allows queuing
  // counter padding combined with data shuffling
  // use signed counters
  addRBTestSet< Tests::RBXXS, ssize_t, 64*1024, sizeof(IntegerWord), 64, SHUFFLE >(functions);
  
  // mask all signals here, and in all child threads (they will be handled in the SignalHandler's thread)
  Elucidate::Sys::SignalHandler sh;
  Elucidate::Sys::coreLock();
  
  for (auto const& it: tests) {
    auto c = functions[it]();
    
    if (metrics) {
      auto t = std::get< 0 >(it);
      auto intrusive = std::get< 1 >(it), pause = std::get< 2 >(it);
      if (intrusive) {
        if (pause) {
            std::cerr << "pws:";
        } else {
          std::cerr << "int:";
      }} else {
        std::cerr << testName(t) << ":";
      }
      printResult(c);
      if (intrusive) {
        log_histogram(std::get< 10 >(c), testName(t) + "Intrusive"s + (pause ? "Pause"s : ""s) + ".csv");
      } // else histogram will have no data
    }
  }

  return 0;
}

