# C++ 并发编程（二）：Mutex（互斥锁）


多个线程访问同一资源时，为了保证数据的一致性，最简单的方式就是使用 mutex（互斥锁）。

引用 cppreference 的介绍：
> The mutex class is a synchronization primitive that can be used to protect shared data from being simultaneously accessed by multiple threads. 

## Mutex 1

直接操作 mutex，即直接调用 mutex 的 `lock / unlock` 函数。

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>


std::mutex g_mutex;
int g_count = 0;

void Counter() {
  g_mutex.lock();

  int i = ++g_count;
  std::cout << "count: " << i << std::endl;

  // 前面代码如有异常，unlock 就调不到了。
  g_mutex.unlock();
}

int main() {
  const std::size_t SIZE = 4;

  // 创建一组线程。
  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Counter);
  }

  // 等待所有线程结束。
  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}
```
可惜的是，STL 没有提供 `boost::thread_group` 这样代表一组线程的工具，通过 `std::vector` 固然也能达到目的，但是代码不够简洁。

## Mutex 2

使用 `lock_guard` 自动加锁、解锁。原理是 RAII，和智能指针类似。

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex g_mutex;
int g_count = 0;

void Counter() {
  // lock_guard 在构造函数里加锁，在析构函数里解锁。
  std::lock_guard<std::mutex> lock(g_mutex);

  int i = ++g_count;
  std::cout << "count: " << i << std::endl;
}

int main() {
  const std::size_t SIZE = 4;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Counter);
  }

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}
```

## Mutex 3

使用 `unique_lock` 自动加锁、解锁。
`unique_lock` 与 `lock_guard` 原理相同，但是提供了更多功能（比如可以结合条件变量使用）。
注意：`mutex::scoped_lock` 其实就是 `unique_lock<mutex>` 的 `typedef`。

至于 `unique_lock` 和 `lock_guard` 详细比较，可移步 [StackOverflow](http://stackoverflow.com/questions/6731027/boostunique-lock-vs-boostlock-guard)。

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex g_mutex;
int g_count = 0;

void Counter() {
  std::unique_lock<std::mutex> lock(g_mutex);

  int i = ++g_count;
  std::cout << "count: " << i << std::endl;
}

int main() {
  const std::size_t SIZE = 4;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Counter);
  }

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}
```

## Mutex 4

为输出流使用单独的 mutex。
这么做是因为 IO 流并不是线程安全的！
如果不对 IO 进行同步，此例的输出很可能变成：
```
count == count == 2count == 41
count == 3
```
因为在下面这条输出语句中：
```cpp
std::cout << "count == " << i << std::endl;
```
输出 "count == " 和 i 这两个动作不是原子性的（atomic），可能被其他线程打断。

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex g_mutex;
std::mutex g_io_mutex;
int g_count = 0;

void Counter() {
  int i;
  {
    std::unique_lock<std::mutex> lock(g_mutex);
    i = ++g_count;
  }

  {
    std::unique_lock<std::mutex> lock(g_io_mutex);
    std::cout << "count: " << i << std::endl;
  }
}

int main() {
  const std::size_t SIZE = 4;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Counter);
  }

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}
```
