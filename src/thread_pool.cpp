#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"

// Boost doc of io_context::run says:
// Multiple threads may call the run() function to set up a pool of threads
// from which the io_context may execute handlers. All threads that are waiting
// in the pool are equivalent and the io_context may choose any one of them to
// invoke a handler.

// See also:
// http://progsch.net/wordpress/?p=71
// http://stackoverflow.com/questions/17156541/why-do-we-need-to-use-boostasioio-servicework

class ThreadPool {
public:
  explicit ThreadPool(std::size_t size)
      : work_guard_(boost::asio::make_work_guard(io_context_)) {
    workers_.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
      workers_.emplace_back(&boost::asio::io_context::run, &io_context_);
    }
  }

  ~ThreadPool() {
    io_context_.stop();

    for (auto& w : workers_) {
      w.join();
    }
  }

  // Add new work item to the pool.
  template<class F>
  void Enqueue(F f) {
    boost::asio::post(io_context_, f);
  }

private:
  std::vector<std::thread> workers_;
  boost::asio::io_context io_context_;

  typedef boost::asio::io_context::executor_type ExecutorType;
  boost::asio::executor_work_guard<ExecutorType> work_guard_;
};

// For output.
std::mutex g_io_mutex;

int main() {
  unsigned int cores = std::thread::hardware_concurrency();
  std::cout << "hardware concurrency: " << cores << std::endl;

  // Create a thread pool of 4 worker threads.
  ThreadPool pool(4);

  // Queue a bunch of work items.
  for (int i = 0; i < 4; ++i) {
    pool.Enqueue([i] {
      {
        std::lock_guard<std::mutex> lock(g_io_mutex);
        std::cout << "Hello" << "(" << i << ") " << std::endl;
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));

      {
        std::lock_guard<std::mutex> lock(g_io_mutex);
        std::cout << "World" << "(" << i << ")" << std::endl;
      }
    });
  }

  return 0;
}

// Sample output:
// Hello(0)
// Hello(1)
// Hello(2)
// Hello(3)
// <Wait about 1 second>
// World(3)
// World(2)
// World(1)
// World(0)
