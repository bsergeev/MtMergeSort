#include <array>
#include <cassert>
#include <chrono>
#include <iostream>
#include <future>
#include <stack>
#include <vector>

namespace util {
template<typename TimeT = std::chrono::microseconds,
    typename ClockT = std::chrono::high_resolution_clock,
    typename DurationT = double>
    class Stopwatch
{
public:
    Stopwatch() { start(); }
    void start() { _start = _end = ClockT::now(); }
    DurationT stop() { _end = ClockT::now();  return elapsed(); }
    DurationT elapsed() {
        auto delta = std::chrono::duration_cast<TimeT>(_end - _start);
        return static_cast<DurationT>(delta.count());
    }
private:
    std::chrono::time_point<ClockT> _start, _end;
};
} // namespace util


static const size_t N =
#ifdef NDEBUG
10'000'000;
#else
7;
#endif
using value_t = float;

// Merges two sections of A[]: A[l..m] and A[m+1..r] 
void merge(std::array<value_t, N>& A, size_t l, size_t m, size_t r) { 
  assert(l <= m && m <= r);
  const size_t n1 = m - l + 1;
  const size_t n2 = r - m;

  // Copy data to left and right temp arrays, AL[] and AR[]
  std::vector<value_t> AL(n1), AR(n2);
  for (size_t i = 0; i < n1; ++i) {
    AL[i] = A[l + i];
  }
  for (size_t i = 0; i < n2; ++i) {
    AR[i] = A[m + 1 + i];
  }
  
  // Merge the temp arrays back into A[l..r]
  size_t i = 0, j = 0, k = l;
  while (i < n1 && j < n2) {
    A[k++] = (AL[i] <= AR[j])? AL[i++] : AR[j++];
  }

  // Copy the remaining elements of AL[], if any
  while (i < n1) {
    A[k++] = AL[i++];
  }

  // Copy the remaining elements of AR[], if any
  while (j < n2) {
    A[k++] = AR[j++];
  }
}

// Below is non-recursive implementation of this classic recursive one:
//void mergeSort(std::array<value_t, N>& arr, size_t l, size_t r) {
//  if (l < r) {
//    const size_t m = l + (r - l) / 2; // = (l + r)/2
//    mergeSort(arr, l, m);
//    mergeSort(arr, m + 1, r);
//    merge(arr, l, m, r);
//  }
//} 
void mergeSort(std::array<value_t, N>& arr, size_t l0, size_t r0) {
  struct MergeSortParams {
    size_t l, r;
    size_t m;
    size_t location;
  };
  std::stack<MergeSortParams> stk;
  stk.push({ l0, r0, 0, 1 });
  while (!stk.empty()) {
    const auto& stkTop = stk.top();
    const size_t l = stkTop.l;
    const size_t r = stkTop.r;
          size_t m = stkTop.m;
    const size_t location = stkTop.location;
    stk.pop();

    if (l < r) {
      switch (location) {
      case 1:
        m = (l + r) / 2;
        stk.push({ l, r, m, 2 });
        stk.push({ l, m, m, 1 });
        break;
      case 2:
        stk.push({ l, r, m, 3 });
        stk.push({ m+1, r, m, 1 });
        break;
      case 3:
        merge(arr, l, m, r);
        break;
      }
    }
  }
} 
  
int main() 
{ 
  auto pArr = std::make_unique<std::array<value_t, N>>();
  std::array<value_t, N>& arr = *(pArr.get());

#ifdef NDEBUG
  for (size_t i = 0; i < N; ++i) {
    arr[i] = rand() / static_cast<value_t>(RAND_MAX);
  }
#else
  size_t i = 0;
  for (auto a : std::array<value_t, N>{38, 27, 43, 3, 9, 82, 10}) {
    arr[i++] = a;
  }
#endif
  
  static const size_t N_THREADS = 4;
  static_assert(N > N_THREADS, "N is too small!");
  std::array<size_t, N_THREADS + 1> n;
  for (size_t i = 0; i < N_THREADS; ++i) {
    n[i] = i * N / N_THREADS;
  }
  n[N_THREADS] = N;

  std::array<std::future<void>, N_THREADS> futures;
  
  util::Stopwatch<> stopwatch;
#if 0
  // Single-threaded sort (about 37.3 seconds)
  mergeSort(arr, 0, N - 1);
#else
  // Sort on 4 + 2 threads (about 10.6 second)
  for (size_t i = 0; i < N_THREADS; ++i) {
    futures[i] = std::async(std::launch::async, 
      [&arr, l=n[i], r=n[i+1]-1] { mergeSort(arr, l, r); });
  }

  std::array<std::future<void>, N_THREADS/2> futures2;
  for (size_t i = 0; i < N_THREADS/2; ++i) {
    futures2[i] = std::async(std::launch::async,
      [&arr, &futures, i2=2*i, l=n[2*i], m=n[2*i+1]-1, r=n[2*i+2]-1] {
        futures[i2].get(); futures[i2 + 1].get();
        merge(arr, l, m, r);
    });
  }
  futures2[0].get();  futures2[1].get();
  merge(arr, n[0], n[2]-1, n[4]-1);
#endif
  const auto elapsed = stopwatch.stop();
  
  const bool sorted = [&arr]() {
    auto prev = arr[0];
    for (size_t i = 1; i < N; ++i)
      if (prev > arr[i])
        return false;
    return true;
  }();
  std::cout << "Sorted " << (sorted ? "successfully" : "UNSUCCESSFULLY") 
            << " in " << elapsed / 1'000'000 << " seconds" << std::endl;

#ifndef NDEBUG
  for (auto a : arr) std::cout << a << std::endl;
#endif
  return 0;
}