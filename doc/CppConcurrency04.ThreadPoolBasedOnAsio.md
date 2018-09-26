# C++ 并发编程（四）：基于 Asio 的线程池

目前项目中使用的线程池（详见：http://threadpool.sourceforge.net/），虽然能用，但是代码复杂且很久没有人维护了。

本文结合 Thread 和 Asio，实现了一个线程池。一二十行代码，不能更简单了！

头文件：
```cpp
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#define BOOST_ASIO_NO_DEPRECATED
#include "boost/asio.hpp"
```

线程池类：
```cpp
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
```
成员变量 `work_guard_` 的作用是，让 `io_context` 即使在没有异步任务可执行时也保持运行（即 `io_context::run` 不返回）。详见 StackOverflow 的讨论：[Why should I use io_service::work?][1]

示例：
```cpp
// For output.
std::mutex g_io_mutex;

int main() {
  // Create a thread pool of 4 worker threads.
  ThreadPool pool(4);

  // Queue a bunch of work items.
  for (int i = 0; i < 8; ++i) {
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
```

输出（每次都不一样）：
```
Hello(0)
Hello(1)
Hello(2)
Hello(3)
<Wait about 1 second>
World(3)
World(2)
World(1)
World(0)
```

参考：
- [A Thread Pool with Boost.Threads and Boost.Asio][2]
- [Why do we need to use boost::asio::io_service::work?][3]


  [1]: http://stackoverflow.com/questions/13219296/why-should-i-use-io-servicework
  [2]: http://progsch.net/wordpress/?p=71
  [3]: http://stackoverflow.com/questions/17156541/why-do-we-need-to-use-boostasioio-servicework