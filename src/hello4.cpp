#include <iostream>
#include <boost/thread.hpp>

// Create a thread by a member function.

class Hello {
public:
  Hello() {
    boost::thread hello_thread(boost::bind(&Hello::Entry, this, "World"));
    hello_thread.join();
  }

private:
  void Entry(const char* what) {
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    std::cout << "Hello, " << what << "!" << std::endl;
  }
};

int main() {
  Hello hello;

  return 0;
}