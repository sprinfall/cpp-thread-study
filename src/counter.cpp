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

  // Wait for the detached t2 to finish.
  std::this_thread::sleep_for(std::chrono::seconds(4));

  return 0;
}
