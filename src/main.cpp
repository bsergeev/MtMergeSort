
#include <array>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <future>
#include <stack>
#include <string>
#include <vector>

// To build single-threaded code, change SINGLE_THREADED to true:
constexpr bool SINGLE_THREADED = false;
static const size_t N = 10'000'000; // size of the array to sort
using value_t = float;

// The following are only for multi-threaded code
//#define FORCE_ASYNC
#undef FORCE_ASYNC
constexpr size_t N_LEVELS = 2; // 1 level -> 2 threads, 2 -> 4 threads, 3 -> 8 threads
constexpr size_t MAX_THREADS = 1 << N_LEVELS;
static_assert(N > MAX_THREADS, "N is too small!");

constexpr std::array<size_t, MAX_THREADS + 1> ComputeArrIndices() noexcept {
  std::array<size_t, MAX_THREADS + 1> n{};
  for (size_t i = 0; i < MAX_THREADS; ++i) {
    n[i] = i * N / MAX_THREADS;
  }
  n[MAX_THREADS] = N;
  return n;
}

namespace util {
template<typename TimeT  = std::chrono::microseconds,
         typename ClockT = std::chrono::high_resolution_clock,
         typename DurationT = double>
class Stopwatch {
public:
  Stopwatch() noexcept { start(); }
  void start() noexcept { _start = _end = ClockT::now(); }
  DurationT stop() noexcept { _end = ClockT::now();  return elapsed(); }
  DurationT elapsed() const { return static_cast<DurationT>(std::chrono::duration_cast<TimeT>(_end-_start).count()); }
private:
  std::chrono::time_point<ClockT> _start, _end;
};
} // namespace util

// Merges two sections of A[]: A[l..m] and A[m+1..r] 
void merge(std::array<value_t, N>& A, size_t l, size_t m, size_t r) { 
  assert(l <= m && m <= r);
  const size_t n1 = m - l + 1;
  const size_t n2 = r - m;

  // Copy data to left and right temp arrays, AL[] and AR[]
  std::vector<value_t> AL(n1), AR(n2);

  // Replacing loops with memcpy makes no discernible difference (optimizing
  // compilers must be doing this).
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

#if 0
  // Copy the remaining elements of AL[], if any
  while (i < n1) {
    A[k++] = AL[i++];
  }
  // Copy the remaining elements of AR[], if any
  while (j < n2) {
    A[k++] = AR[j++];
  }
#else
  if (i < n1) {
    std::memcpy(&A[k], &AL[i], (n1-i)*sizeof(value_t));
  }
  if (j < n2) {
    std::memcpy(&A[k], &AR[j], (n2-j)*sizeof(value_t));
  }
#endif
}

void mergeSort(std::array<value_t, N>& arr, size_t l, size_t r) {
  if (l < r) {
    const size_t m = l + (r - l) / 2; // = (l + r)/2
    mergeSort(arr, l, m);
    mergeSort(arr, m + 1, r);
    merge(arr, l, m, r);
  }
} 

int main() 
{ 
  std::cout << "Sorting "<< N <<" floats on "<< (SINGLE_THREADED? "single thread" : std::to_string(MAX_THREADS)+" threads") << std::endl;

  // Allocate the array on the heap (to avoid stack overflow)
  auto pArr = std::make_unique<std::array<value_t, N>>();
  auto& arr = *(pArr.get());

  // Random initialize the array
  for (size_t i = 0; i < N; ++i) {
    arr[i] = rand() / static_cast<value_t>(RAND_MAX);
  }
  
  util::Stopwatch<> stopwatch;

  if constexpr (SINGLE_THREADED) {
    mergeSort(arr, 0, N - 1);
  } 
  else // multiple threads
  {
    // Build indices of sub-arrays for the highest level. 
    // For lower levels, it'll be traversed with `stride` depending on the level.
    static const std::array<size_t, MAX_THREADS + 1> n = ComputeArrIndices();

    static std::array<std::future<void>, MAX_THREADS> sort_future;
    if constexpr (N_LEVELS == 1) //---------------------------------------------
    {
      sort_future[0] = std::async([&arr] { mergeSort(arr, n[0], n[1]-1); });
      sort_future[1] = std::async([&arr] { mergeSort(arr, n[1], n[2]-1); });
    }
    else if constexpr (N_LEVELS == 2) //----------------------------------------
    {
      // Sort four sub-arrays
      static auto sort_future0 = std::async([&arr] { 
        #ifdef FORCE_ASYNC
        std::launch::async,
        #endif
        mergeSort(arr, n[0], n[1]-1); });
      static auto sort_future1 = std::async([&arr] { 
        #ifdef FORCE_ASYNC
        std::launch::async,
        #endif
        mergeSort(arr, n[1], n[2]-1); });
      static auto sort_future2 = std::async([&arr] { 
        #ifdef FORCE_ASYNC
        std::launch::async,
        #endif
        mergeSort(arr, n[2], n[3]-1); });
      static auto sort_future3 = std::async([&arr] { 
        #ifdef FORCE_ASYNC
        std::launch::async,
        #endif
        mergeSort(arr, n[3], n[4]-1); });

      // Merge two sub-arrays
      sort_future[0] = std::async(
        #ifdef FORCE_ASYNC
        std::launch::async,
        #endif
        [&arr] { sort_future0.get(); sort_future1.get(); merge(arr, n[0], n[1]-1, n[2]-1); 
      });
      sort_future[1] = std::async(
        #ifdef FORCE_ASYNC
        std::launch::async,
        #endif
        [&arr] { sort_future2.get(); sort_future3.get(); merge(arr, n[2], n[3]-1, n[4]-1);
      });
    }
    else if constexpr (N_LEVELS > 2) // 8 or more threads ----------------------
    {
      // Sort sub-arrays
      for (size_t i = 0; i < MAX_THREADS; ++i) {
        sort_future[i] = std::async(
          #ifdef FORCE_ASYNC
          std::launch::async,
          #endif
          [&arr, i] { mergeSort(arr, n[i], n[i + 1]-1); });
      }

      // Process each level by spawning worker threads to merge sub-arrays
      for (size_t level = N_LEVELS; level > 0; --level) {
        constexpr size_t one = 1;
        const size_t N_threads = one << level;           // # of threads for this level
        const size_t stride = one << (N_LEVELS - level); // stride for indices in `n` 
  
        // On 1st pass, wait for mergeSort() futures; later--on merge() futures from previous pass
        const size_t N_to_wait = (level == N_LEVELS)? MAX_THREADS : 2 * N_threads;
        for (size_t i = 0; i < N_to_wait; ++i) {
          sort_future[i].get();
        }

        // Merge sub-arrays
        for (size_t i = 0; i < N_threads; ++i) {
          sort_future[i] = std::async(
            #ifdef FORCE_ASYNC
            std::launch::async,
            #endif
            [&arr, i, stride] { // stride is a power of 2 => stride/2 below is integer
              merge(arr, n[stride*i], n[stride*i + stride/2]-1, n[stride*(i+1)]-1);
            });
        }
      } // level loop
    } //------------------------------------------------------------------------
    sort_future[0].get();  
    sort_future[1].get();
    merge(arr, n[0], n[MAX_THREADS/2] - 1, n[MAX_THREADS] - 1);
  }

  const auto elapsed_uS = stopwatch.stop();
  
  // Check that the array is indeed sorted
  const bool sorted = [&arr]() {
    for (size_t i = 1; i < N; ++i) 
      if (arr[i - 1] > arr[i]) return false;
    return true;
  }();
  std::cout << "Sorted " << (sorted ? "successfully" : "UNSUCCESSFULLY") 
            << " in " << elapsed_uS / 1'000'000 << " seconds" << std::endl;
  return (sorted)? 0 : 13;
}