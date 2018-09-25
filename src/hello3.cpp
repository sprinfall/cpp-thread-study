#include <iostream>
#include <thread>

// Create a thread by a function object (functor).

class Hello {
public:
  void operator()(const char* what) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello, " << what << "!" << std::endl;
  }
};

int main() {
  Hello hello;

  // Copy the thread object.
  std::thread t1(hello, "World");
  t1.join();

  // Don't copy the thread object, use std::ref to pass in a reference.
  // But the user must ensure that the referred-to object outlives the
  // newly-created thread of execution.
  std::thread t2(std::ref(hello), "World");
  t2.join();

  return 0;
}