# C++ 并发编程（六）：信号量（Semaphore）

下面这段介绍，修改自 wxWidgets 官方文档（详见：[wxSemaphore Class Reference][1]）。

> **Semaphore** is a counter limiting the number of threads concurrently accessing a shared resource.

> This counter is always between 0 and the maximum value specified during the semaphore creation. When the counter is strictly greater than 0, a call to `Wait` returns immediately and decrements the counter. As soon as it reaches 0, any subsequent calls to Semaphore::Wait block and only return when the semaphore counter becomes strictly positive again as the result of calling `Signal` which increments the counter.

> In general, semaphores are useful to restrict access to a shared resource which can only be accessed by some fixed number of clients at the same time. For example, when modeling a hotel reservation system a semaphore with the counter equal to the total number of available rooms could be created. Each time a room is reserved, the semaphore should be acquired by calling `Wait` and each time a room is freed it should be released by calling `Signal`.

C++11 和 Boost.Thread 都没有提供信号量。对此 Boost 是这样解释的（[Why has class semaphore disappeared?][2]）：

> Semaphore was removed as too error prone. The same effect can be achieved with greater safety by the combination of a mutex and a condition variable. Dijkstra (the semaphore's inventor), Hoare, and Brinch Hansen all depreciated semaphores and advocated more structured alternatives. In a 1969 letter to Brinch Hansen, Wirth said "semaphores ... are not suitable for higher level languages." [Andrews-83] summarizes typical errors as "omitting a **P** or a **V**, or accidentally coding a **P** on one semaphore and a **V** on on another", forgetting to include all references to shared objects in critical sections, and confusion caused by using the same primitive for "both condition synchronization and mutual exclusion".

简单来说，就是信号量太容易出错了（too error prone），通过组合互斥锁（mutex）和条件变量（condition variable）可以达到相同的效果，且更加安全。实现如下：

```cpp
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
    cv_.wait(lock, [=] { return count_ > 0; });
    --count_;
  }

private:
  std::mutex mutex_;
  std::condition_variable cv_;
  int count_;
};
```

下面创建三个工作线程（Worker），来测试这个信号量。
```cpp
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
```
每个工作线程先等待信号量，然后输出线程 ID 和当前时间，输出操作以互斥锁同步以防止错位，睡眠一秒是为了模拟线程处理数据的耗时。
```cpp
std::mutex g_io_mutex;

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
```

信号量本身是一个全局对象，`count` 为 `1`，一次只允许一个线程访问：
```cpp
Semaphore g_semaphore(1);
```
输出为：
```plain
Thread 1d38: wait succeeded (13:10:10)
Thread 20f4: wait succeeded (13:10:11)
Thread 2348: wait succeeded (13:10:12)
```
可见每个线程相隔一秒，即一次只允许一个线程访问。如果把 `count` 改为 `3`：
```cpp
Semaphore g_semaphore(3);
```
那么三个线程输出的时间应该一样：
```plain
Thread 19f8: wait succeeded (13:10:57)
Thread 2030: wait succeeded (13:10:57)
Thread 199c: wait succeeded (13:10:57)
```

最后附上 `FormatTimeNow` 函数的实现：
```cpp
std::string FormatTimeNow(const char* format) {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm* now_tm = std::localtime(&now_c);

  char buf[20];
  std::strftime(buf, sizeof(buf), format, now_tm);
  return std::string(buf);
}
```

参考：
- [C++0x has no semaphores? How to synchronize threads?][3]
- [Semaphore (programming)][4]


  [1]: http://docs.wxwidgets.org/trunk/classwx_semaphore.html
  [2]: http://www.boost.org/doc/libs/1_31_0/libs/thread/doc/faq.html
  [3]: http://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads
  [4]: https://en.wikipedia.org/wiki/Semaphore_%28programming%29#Semaphore_vs._mutex
