#include <iostream>
#include <boost/thread.hpp>

// Create a thread by a function with argument(s).

void Hello(const char* what) {
  // Sleep 1 second to simulate the data processing.
  boost::this_thread::sleep(boost::posix_time::seconds(1));
  std::cout << "Hello, " << what << "!" << std::endl;
}

int main() {
  boost::thread hello_thread(Hello, "World");
  // Equivalent to:
  // boost::thread hello_thread(boost::bind(&Hello, "World"));

  hello_thread.join();

  return 0;
}