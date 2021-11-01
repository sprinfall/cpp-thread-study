#include <chrono>
#include <condition_variable>
#include <ctime>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Implement semaphore based on mutex and condition variable.

// Adapted from:
// http://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

// Boost: Why has class semaphore disappeared?
// http://www.boost.org/doc/libs/1_31_0/libs/thread/doc/faq.html

// https://en.wikipedia.org/wiki/Semaphore_%28programming%29#Semaphore_vs._mutex

class Semaphore {
public:
  explicit Semaphore(int count = 0) : count_(count) {
  }

  void Signal() {
    std::unique_lock<std::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return count_ > 0; });
    --count_;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

Semaphore g_semaphore(3);
std::mutex g_io_mutex;

std::string FormatTimeNow(const char* format) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm* now_tm = std::localtime(&now_c);

  char buf[20];
  std::strftime(buf, sizeof(buf), format, now_tm);
  return std::string(buf);
}

void Worker() {
  g_semaphore.Wait();

  std::thread::id thread_id = std::this_thread::get_id();

  std::string now = FormatTimeNow("%H:%M:%S");
  {
    std::lock_guard<std::mutex> lock(g_io_mutex);
    std::cout << "Thread " << thread_id << ": wait succeeded" << " (" << now << ")" << std::endl;
  }

  // Sleep 1 second to simulate data processing.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  g_semaphore.Signal();
}

int main() {
  const std::size_t SIZE = 3;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Worker);
  }

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}

//Thread 1d38: wait succeeded (13:10:10)
//Thread 20f4: wait succeeded (13:10:11)
//Thread 2348: wait succeeded (13:10:12)
