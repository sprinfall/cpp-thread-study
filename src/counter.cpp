#include <iostream>
#include <boost/thread.hpp>

class Counter {
public:
  Counter(int value) : value_(value) {
  }

  void operator()() {
    while (value_ > 0) {
      std::cout << value_ << " ";
      --value_;
      boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    std::cout << std::endl;
  }

private:
  int value_;
};

int main() {
  boost::thread t1(Counter(3));
  t1.join();

  boost::thread t2(Counter(3));
  t2.detach();

  // Wait for the detached t2 to finish.
  boost::this_thread::sleep(boost::posix_time::seconds(4));

  return 0;
}