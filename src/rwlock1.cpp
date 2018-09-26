#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

// For this example, boost::atomic<> should be a better choice.

class Counter {
public:
  Counter() : value_(0) {
  }

  // Multiple threads/readers can read the counter's value at the same time.
  std::size_t Get() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return value_;
  }

  // Only one thread/writer can increment/write the counter's value.
  void Increase() {
    // You can also use lock_guard here.
    std::unique_lock<std::shared_mutex> lock(mutex_);
    value_++;
  }

  // Only one thread/writer can reset/write the counter's value.
  void Reset() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    value_ = 0;
  }

private:
  mutable std::shared_mutex mutex_;
  std::size_t value_;
};

std::mutex g_io_mutex;

void Worker(Counter& counter) {
  for (int i = 0; i < 3; ++i) {
    counter.Increase();
    std::size_t value = counter.Get();

    std::lock_guard<std::mutex> lock(g_io_mutex);
    std::cout << std::this_thread::get_id() << ' ' << value << std::endl;
  }
}

int main() {
  const std::size_t SIZE = 2;

  Counter counter;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  v.emplace_back(&Worker, std::ref(counter));
  v.emplace_back(&Worker, std::ref(counter));

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}

// Output (Random):
// 2978 1
// 4114 2
// 2978 3
// 4114 4
// 4114 6
// 2978 5