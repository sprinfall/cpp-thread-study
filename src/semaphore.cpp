#include <chrono>
#include <condition_variable>
#include <ctime>
#include <functional>
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
  explicit Semaphore(int count) : count_(count) {
  }

  void Signal() {
    std::unique_lock<std::mutex> lock{ mutex_ };
    ++count_;
    cv_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock{ mutex_ };
    cv_.wait(lock, [this] { return count_ > 0; });
    --count_;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};

std::mutex g_cout_mutex;

std::string GetTimestamp() {
  std::time_t t = std::time(nullptr);
  std::tm* tm = std::localtime(&t);
  char buf[20];
  std::strftime(buf, sizeof(buf), "%H:%M:%S", tm);
  return std::string(buf);
}

void Worker(Semaphore& semaphore) {
  semaphore.Wait();

  std::thread::id thread_id = std::this_thread::get_id();
  std::string timestamp = GetTimestamp();

  g_cout_mutex.lock();
  std::cout << thread_id << ": wait succeeded (" << timestamp << ")"
            << std::endl;
  g_cout_mutex.unlock();

  // Sleep 1 second to simulate data processing.
  std::this_thread::sleep_for(std::chrono::seconds(1));

  semaphore.Signal();
}

int main() {
  // Please try other count values (e.g., 3) and note the timestamp changes in
  // the output.
  Semaphore semaphore{ 1 };

  std::vector<std::thread> v;
  for (std::size_t i = 0; i < 3; ++i) {
    v.emplace_back(std::bind(&Worker, std::ref(semaphore)));
  }

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}

// Output example (if semaphore count == 1):
// 1d38: wait succeeded (13:10:10)
// 20f4: wait succeeded (13:10:11)
// 2348: wait succeeded (13:10:12)
