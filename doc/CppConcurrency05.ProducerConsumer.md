# C++ 并发编程（五）：生产者 - 消费者

生产者 - 消费者（Producer-Consumer），也叫有限缓冲（Bounded-Buffer），是多线程同步的经典问题之一。详见 [Wikipedia][1]。

代码改写自 Boost.Thread 自带的示例（`libs/thread/example/condition.cpp`），以「条件变量」实现同步。

## 头文件
```cpp
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
```

## 有限缓冲类

```cpp
class BoundedBuffer {
public:
  BoundedBuffer(const BoundedBuffer& rhs) = delete;
  BoundedBuffer& operator=(const BoundedBuffer& rhs) = delete;

  BoundedBuffer(std::size_t size)
      : begin_(0), end_(0), buffered_(0), circular_buffer_(size) {
  }

  void Produce(int n) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      // 等待缓冲不为满。
      not_full_cv_.wait(lock, [=] { return buffered_ < circular_buffer_.size(); });

      // 插入新的元素，更新下标。
      circular_buffer_[end_] = n;
      end_ = (end_ + 1) % circular_buffer_.size();

      ++buffered_;
    }  // 通知前，自动解锁。

    // 通知消费者。
    not_empty_cv_.notify_one();
  }

  int Consume() {
    std::unique_lock<std::mutex> lock(mutex_);
    // 等待缓冲不为空。
    not_empty_cv_.wait(lock, [=] { return buffered_ > 0; });

    // 移除一个元素。
    int n = circular_buffer_[begin_];
    begin_ = (begin_ + 1) % circular_buffer_.size();

    --buffered_;

    // 通知前，手动解锁。
    lock.unlock();
    // 通知生产者。
    not_full_cv_.notify_one();
    return n;
  }

private:
  std::size_t begin_;
  std::size_t end_;
  std::size_t buffered_;
  std::vector<int> circular_buffer_;
  std::condition_variable not_full_cv_;
  std::condition_variable not_empty_cv_;
  std::mutex mutex_;
};
```

生产者与消费者线程共享的缓冲。`g_io_mutex` 是用来同步输出的。
```cpp
BoundedBuffer g_buffer(2);
boost::mutex g_io_mutex;
```

## 生产者
生产 100000 个元素，每 10000 个打印一次。
```cpp
void Producer() {
  int n = 0;
  while (n < 100000) {
    g_buffer.Produce(n);
    if ((n % 10000) == 0) {
      std::unique_lock<std::mutex> lock(g_io_mutex);
      std::cout << "Produce: " << n << std::endl;
    }
    ++n;
  }

  g_buffer.Produce(-1);
}
```

## 消费者
每消费到 10000 的倍数，打印一次。
```cpp
void Consumer() {
  std::thread::id thread_id = std::this_thread::get_id();

  int n = 0;
  do {
    n = g_buffer.Consume();
    if ((n % 10000) == 0) {
      std::unique_lock<std::mutex> lock(g_io_mutex);
      std::cout << "Consume: " << n << " (" << thread_id << ")" << std::endl;
    }
  } while (n != -1);  // -1 表示缓冲已达末尾。

  // 往缓冲里再放一个 -1，这样其他消费者才能结束。
  g_buffer.Produce(-1);
}
```

## 主程序

一个生产者线程，三个消费者线程。
```cpp
int main() {
  std::vector<std::thread> threads;

  threads.push_back(std::thread(&Producer));
  threads.push_back(std::thread(&Consumer));
  threads.push_back(std::thread(&Consumer));
  threads.push_back(std::thread(&Consumer));

  for (auto& t : threads) {
    t.join();
  }

  return 0;
}
```

输出（括号中为线程 ID）：
```
Produce: 0
Consume: 0 (13c0)
Produce: 10000
Consume: 10000 (15fc)
Produce: 20000
Consume: 20000 (2558)
Produce: 30000
Consume: 30000 (13c0)
Produce: 40000
Consume: 40000 (15fc)
Produce: 50000
Consume: 50000 (13c0)
Produce: 60000
Consume: 60000 (15fc)
Produce: 70000
Consume: 70000 (13c0)
Produce: 80000
Consume: 80000 (15fc)
Produce: 90000
Consume: 90000 (15fc)
```

## 分析

考虑一个生产者和一个消费者的情形，假定缓冲的大小为 2，来看看三个成员变量如何变化。
```
            buffered_    begin_      end_
 初始           0          0          0
 生产           1          0          1
 消费           0          1          1
 消费          等待 buffered_ > 0 ...
 生产           1          1          0
 ...
```

参考：
- [Wikipedia: Producer–consumer problem][2]
- [StackOverflow: Bounded Buffers (Producer Consumer)][3]
- [StackOverflow: Empty element in array-based bounded buffer][4]


  [1]: https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
  [2]: https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
  [3]: http://stackoverflow.com/questions/9578050/bounded-buffers-producer-consumer
  [4]: http://stackoverflow.com/questions/9517405/empty-element-in-array-based-bounded-buffer
