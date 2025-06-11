#if !defined(ELUCIDATE__UT__BENCHMARKITC_HPP)
#define ELUCIDATE__UT__BENCHMARKITC_HPP

#include <memory>
#include <thread>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <iomanip>
#include <hdr/hdr_histogram.h>

#include "TscClock.hpp"
#include "Thread.hpp"
#include "OSIndex.hpp"
#include "ErrorCode.hpp"

namespace Elucidate::UT {
using SafeClock = std::chrono::high_resolution_clock;
// \todo /sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq
using IntrusiveUnsafeClock = Elucidate::Util::TscClock< 3900000000, false >;
using IntrusiveTimepoint = std::chrono::time_point< IntrusiveUnsafeClock >;

template< typename Buffers, int COUNT >
struct Parms {
  Buffers buffers;
  std::array< long, COUNT > responses;
  int responseCounter;
  using Timepoints = std::array< IntrusiveTimepoint, COUNT >;
  Timepoints tx;
  Timepoints rx;
  std::atomic< int > latchStart, latchEnd;

  Parms()
  :	buffers()
  ,	responses()
  ,	tx()
  ,	rx()
  ,	latchStart(3)
  ,	latchEnd(2)
  {
    for (unsigned int ii = 0; ii < COUNT; ++ii) {
      responses[ii] = 0xdeadbeef;
    }
  }
};

template< int COUNT, int RATE_SCALE, bool Intrusive, typename Elapsed, typename Data >
auto calculateResult(Elapsed const& e, Data const& parms) {
  hdr_histogram* histogram = nullptr;
  // time in nanos => count per microsecond is throughput in 1000,000s/s
  const double rateMps = (parms.responseCounter * double(RATE_SCALE)) 
    / std::chrono::duration< double, std::nano >(e).count();
  const double lossPpm = 1000000.0 * (COUNT - parms.responseCounter) / COUNT;
  if constexpr (!Intrusive) {
    return std::tuple(rateMps, lossPpm, 0l, 0l, 0l, 0l, 0l, 0l, 0l, 0l, histogram);
  }
  
  using Latencies = std::array< IntrusiveUnsafeClock::duration, COUNT >;
  using Intervals = std::array< IntrusiveUnsafeClock::duration, COUNT - 1 >;
  auto latencies(std::make_unique< Latencies >());
  auto intervals(std::make_unique< Intervals >());
  // TODO handle gaps in event of loss without corrupting stats
  for (unsigned ii = 0; ii < COUNT; ++ii) {
    (*latencies)[ii] = parms.rx[ii] - parms.tx[ii];
  }
  for (unsigned ii = 0; ii < (COUNT - 1); ++ii) {
    (*intervals)[ii] = parms.tx[ii + 1] - parms.tx[ii];
  }
  auto totLat = std::accumulate(latencies->begin(), latencies->end(),
    typename IntrusiveUnsafeClock::duration());
  auto avgLat = std::chrono::duration_cast< std::chrono::nanoseconds >(
    totLat).count() / COUNT;
  auto totInt = std::accumulate(intervals->begin(), intervals->end(),
    typename IntrusiveUnsafeClock::duration());
  auto avgInt = std::chrono::duration_cast< std::chrono::nanoseconds >(
    totInt).count() / COUNT;
  auto minLat = std::chrono::duration_cast< std::chrono::nanoseconds >(
    *std::min_element(latencies->begin(), latencies->end())).count();
  auto peaLat = std::chrono::duration_cast< std::chrono::nanoseconds >(
    *std::max_element(latencies->begin(), latencies->end())).count();
  auto minInt = std::chrono::duration_cast< std::chrono::nanoseconds >(
    *std::min_element(intervals->begin(), intervals->end())).count();
  auto peaInt = std::chrono::duration_cast< std::chrono::nanoseconds >(
    *std::max_element(intervals->begin(), intervals->end())).count();
  std::sort(latencies->begin(), latencies->end());
  auto medLat = std::chrono::duration_cast< std::chrono::nanoseconds >(
     (*latencies)[COUNT / 2]).count();
  std::sort(intervals->begin(), intervals->end());
  auto medInt = std::chrono::duration_cast< std::chrono::nanoseconds >(
    (*intervals)[(COUNT - 1) / 2]).count();

  Util::checkReturnAndThrow(0, hdr_init(1, 1000000, 5, &histogram), "hdr_init");
  for (auto const& l : *latencies) {
    hdr_record_value(histogram,
      std::chrono::duration_cast< std::chrono::nanoseconds >(l).count());
  }

  return std::tuple(rateMps, lossPpm, minLat, minInt, avgLat, avgInt,
    medLat, medInt, peaLat, peaInt, histogram);
}

template< typename Worker, typename Data >
struct LatchControlled {
	static void work(Data& data) {
		data.latchStart.fetch_sub(1);
		while (data.latchStart.load()) {} // spin until barrier cleared
		Worker::work(data);
		data.latchEnd.fetch_sub(1);
		while (data.latchEnd.load()) {} // spin until barrier cleared
	}
};

template< typename Data, int COUNT, typename Worker, bool Intrusive, bool Pause >
struct TestDriver {
  static void work(Data& parms) {
    Worker worker(parms);
    for (int ii = 0; ii < COUNT; ++ii) {
      auto val = ii;
      if constexpr (Intrusive) {
        // won't be negative
        parms.tx[typename decltype(parms.tx)::size_type(ii)] = IntrusiveUnsafeClock::now();
      }
      worker(val);
      if constexpr (Pause) {
        // give the other thread enough time to have processed, so we won't busy on the signal
        // gives a closer measure of latency in the absence of this kind of contention
        auto till = IntrusiveUnsafeClock::now() + std::chrono::nanoseconds(200);
        while (IntrusiveUnsafeClock::now() < till)
        {} // busy wait
      }
    }
  }
};

using Result = std::tuple<double, double, long, long, long, long, long, long, long, long, hdr_histogram*>;

// @param Data payload
// @param COUNT number of data to transit
// @param Worker type to instantiate to produce
// @param Partner type to instantiate with partner thread to consume
// @param Intrusive whether the implement inline measurements.
// disturbs system under test by recording times.
// otherwise only gross average data available.
// @param Validator type to instantiate to validate outputs
// @param Pause whether to wait 200ns between productions.
// @param Metrics whether to generate metrics
// allows measurement of latency when the consumer is given time to become idle.
// allows measurement of cost of producer inclusive of pause.
template< typename Data, int COUNT, typename Worker, typename Partner, bool Intrusive, typename Validator, bool Pause, bool Metrics, bool Hiperf = true, int RATE_SCALE = 1000 >
auto benchmarkITC() {
  auto parmsP = std::make_unique< Data >();
  auto& parms = *parmsP;
  if constexpr (Intrusive) {
    parms.tx.fill(IntrusiveTimepoint());
    parms.rx.fill(IntrusiveTimepoint());
  }
  std::thread partner(LatchControlled< Partner, Data >::work, std::ref(parms));
  if constexpr (Hiperf) {
    Elucidate::Sys::setHighPriority(partner);
    Elucidate::Sys::setCPURange(partner, Elucidate::Sys::OSIndex{2});
  }
  std::thread driver(LatchControlled< Worker, Data >::work, std::ref(parms));
  if constexpr (Hiperf) {
    Elucidate::Sys::setHighPriority(driver);
    Elucidate::Sys::setCPURange(driver, Elucidate::Sys::OSIndex{1});
  }

  parms.latchStart.fetch_sub(1);
  const auto start = SafeClock::now();
  while (parms.latchEnd.load()) {} // spin until barrier cleared
  const auto end = SafeClock::now();

  driver.join();
  partner.join();

  for (unsigned ii = 0; ii < COUNT; ++ii) {
    if (parms.responses[ii] != Validator()(ii)) {
      std::cerr << "wrong answer! " << parms.responses[ii]
        << " at index " << ii << '\n';
    }
  }
  
  if constexpr (Metrics) {
    return calculateResult< COUNT, RATE_SCALE, Intrusive >(end - start, parms);
  } else {
    return std::tuple(double(), double(), long(), long(), long(), long(),
      long(), long(), long(), long(), nullptr);
  }
}

template< int RATE_SCALE = 1000, typename Tuple >
void printResult(Tuple const& t) {
  std::cerr 
    << "\t" << std::fixed << std::setprecision(1) << std::get< 0 >(t) << (RATE_SCALE == 1000 ? "M" : "k")
    << "\t" << std::get< 1 >(t) << "ppm"
    << "\t" << std::get< 2 >(t) << "ns"
    << "\t" << std::get< 3 >(t) << "ns"
    << "\t" << std::get< 4 >(t) << "ns"
    << "\t" << std::get< 5 >(t) << "ns"
    << "\t" << std::get< 6 >(t) << "ns"
    << "\t" << std::get< 7 >(t) << "ns"
    << "\t" << std::get< 8 >(t) << "ns"
    << "\t" << std::get< 9 >(t) << "ns"
    << "\n";
}

void log_histogram(hdr_histogram* histogram, std::string const& filename) {
  FILE* file = fopen(filename.c_str(), "w");
  Util::checkForBadReturn(nullptr, file, "fopen");
  hdr_percentiles_print(histogram, file, 5, 1.0, CSV); 
  fclose(file);
}

}
#endif

