#include <iostream>
#include <boost/thread.hpp>

// For this example, boost::atomic<> should be a better choice.

class ThreadSafeCounter {
public:
  ThreadSafeCounter() = default;

  // Multiple threads/readers can read the counter's value at the same time.
  size_t Get() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex_);
    return value_;
  }

  // Only one thread/writer can increment/write the counter's value.
  void Increase() {
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
  size_t value_ = 0;
};

boost::mutex io_mutex;

int main() {
  ThreadSafeCounter counter;

  auto increment_and_print = [&counter]() {
    for (int i = 0; i < 3; i++) {
      counter.Increase();
      size_t value = counter.Get();

      io_mutex.lock();
      std::cout << boost::this_thread::get_id() << ' ' << value << std::endl;
      io_mutex.unlock();
    }
  };

  boost::thread thread1(increment_and_print);
  boost::thread thread2(increment_and_print);

  thread1.join();
  thread2.join();
}

// Output:
// 27a8 1
// 27a8 3
// 2434 2
// 27a8 4
// 2434 5
// 2434 6