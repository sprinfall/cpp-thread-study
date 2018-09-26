# C++ 并发编程（一）：创建线程

这个系列是我近期学习 C++ 并发编程的总结，文章和代码最初都是基于 Boost.Thread，但是最近越来越发现，STL 内置的线程和同步工具已经足够完善了。

STL 和 Boost 线程，在设计和用法上极其相似，一旦掌握了一个，不难切换到另一个。如果非要比较的话，Boost 更完善一些，比如 Boost 提供了 `thread_group` 和 `upgrade_lock`，STL 则没有。

此节介绍「线程的创建」。

## Hello 1

通过一个不带参数的函数创建线程。

```cpp
#include <iostream>
#include <thread>

void Hello() {
  // 睡眠一秒以模拟数据处理。
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Hello, World!" << std::endl;
}

int main() {
  // 创建一个线程对象，注意函数 Hello 将立即运行。
  std::thread t(&Hello);

  // 等待线程结束。
  // 否则线程还没执行（完），主程序就已经结束了。
  t.join();

  return 0;
}
```

## Hello 2

通过一个带参数的函数创建线程。

```cpp
#include <iostream>
#include <thread>

void Hello(const char* what) {
  // 睡眠一秒以模拟数据处理。
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Hello, " << what << "!" << std::endl;
}

int main() {
  std::thread t(&Hello, "World");
  
  // 等价于使用 bind：
  //   std::thread t(std::bind(&Hello, "World"));

  t.join();

  return 0;
}
```

## Hello 3

通过一个函数对象——即仿函数（functor）——创建线程。

```cpp
#include <iostream>
#include <thread>

class Hello {
public:
  void operator()(const char* what) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello, " << what << "!" << std::endl;
  }
};

int main() {
  Hello hello;

  // 方式一：拷贝函数对象。
  std::thread t1(hello, "World");
  t1.join();

  // 方式二：不拷贝函数对象，通过 boost::ref 传入引用。
  // 用户必须保证被线程引用的函数对象，拥有超出线程的生命期。
  // 比如这里通过 join 线程保证了这一点。 
  std::thread t2(std::ref(hello), "World");
  t2.

  return 0;
}
```

## Hello 4

通过一个成员函数创建线程。
与前例不同之处在于，需要以 `bind` 绑定 `this` 指针作为第一个参数。

```cpp
#include <iostream>
#include <thread>

class Hello {
public:
  Hello() {
    std::thread t(std::bind(&Hello::Entry, this, "World"));
    t.join();
  }

private:
  // 线程函数
  void Entry(const char* what) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello, " << what << "!" << std::endl;
  }
};

int main() {
  Hello hello;
  return 0;
}
```

## Counter

创建两个线程，各自倒着计数。
此例顺带演示了 detached 线程，被 detached 的线程，自生自灭，不受控制，无法再 join。

```cpp
#include <iostream>
#include <thread>

class Counter {
public:
  Counter(int value) : value_(value) {
  }

  void operator()() {
    while (value_ > 0) {
      std::cout << value_ << " ";
      --value_;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << std::endl;
  }

private:
  int value_;
};

int main() {
  std::thread t1(Counter(3));
  t1.join();

  std::thread t2(Counter(3));
  t2.detach();

  // 等待几秒，不然 t2 根本没机会执行。
  std::this_thread::sleep_for(std::chrono::seconds(4));
  
  return 0;
}
```
