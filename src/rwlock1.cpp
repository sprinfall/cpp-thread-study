#include <iostream>
#include <boost/thread.hpp>

// For this example, boost::atomic<> should be a better choice.

class Counter {
public:
  Counter() : value_(0) {
  }

  // Multiple threads/readers can read the counter's value at the same time.
  size_t Get() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return value_;
  }

  // Only one thread/writer can increment/write the counter's value.
  void Increase() {
    // You can also use lock_guard here.
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    value_++;
  }

  // Only one thread/writer can reset/write the counter's value.
  void Reset() {
    boost::unique_lock<boost::shared_mutex> lock(mutex_);
    value_ = 0;
  }

private:
  mutable boost::shared_mutex mutex_;
  size_t value_;
};

boost::mutex g_io_mutex;

void Worker(Counter& counter) {
  for (int i = 0; i < 3; ++i) {
    counter.Increase();
    size_t value = counter.Get();

    boost::lock_guard<boost::mutex> lock(g_io_mutex);
    std::cout << boost::this_thread::get_id() << ' ' << value << std::endl;
  }
}

int main() {
  Counter counter;

  boost::thread_group threads;
  threads.create_thread(boost::bind(Worker, boost::ref(counter)));
  threads.create_thread(boost::bind(Worker, boost::ref(counter)));

  threads.join_all();
  return 0;
}

// Output (Random):
// 2978 1
// 4114 2
// 2978 3
// 4114 4
// 4114 6
// 2978 5