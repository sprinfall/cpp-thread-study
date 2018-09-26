# C++ 并发编程（七）：读写锁（Read-Write Lock）

STL 和 Boost 都提供了 `shared_mutex` 来解决「读者-写者」问题。`shared_mutex` 这个名字并不十分贴切，不如 pthread 直呼「读写锁」。

所谓「读写锁」，就是同时可以被多个读者拥有，但是只能被一个写者拥有的锁。而所谓「多个读者、单个写者」，并非指程序中只有一个写者（线程），而是说不能有多个写者同时去写。

下面看一个计数器的例子。
```cpp
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
```
`shared_mutex` 比一般的 `mutex` 多了函数 `lock_shared() / unlock_shared()`，允许多个（读者）线程同时加锁、解锁，而 `shared_lock` 则相当于共享版的 `lock_guard`。

对 `shared_mutex` 使用 `lock_guard` 或 `unique_lock` 就达到了写者独占的目的。

测试代码：
```cpp
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
```

输出（仍然是随机性的）：
```
2978 1
4114 2
2978 3
4114 4
4114 6
2978 5
```

当然，对于计数器来说，原子类型 `std::atomic<>` 也许是更好的选择。

假如一个线程，先作为读者用 `shared_lock` 加锁，读完后突然又想变成写者，该怎么办？

**方法一**：先解读者锁，再加写者锁。这种做法的问题是，一解一加之间，其他写者说不定已经介入并修改了数据，那么当前线程作为读者时所持有的状态（比如指针、迭代器）也就不再有效。

**方法二**：用 `upgrade_lock`（仅限 Boost，STL 未提供），可以当做 `shared_lock` 用，但是必要时可以直接从读者「升级」为写者。

```cpp
{
  // Acquire shared ownership to read.
  boost::upgrade_lock<boost::shared_mutex> upgrade_lock(shared_mutex_);

  // Read...

  // Upgrade to exclusive ownership to write.
  boost::upgrade_to_unique_lock<boost::shared_mutex> unique_lock(upgrade_lock);

  // Write...
}
```
可惜的是，我没能给 `upgrade_lock` 找到一个颇具实际意义的例子。
