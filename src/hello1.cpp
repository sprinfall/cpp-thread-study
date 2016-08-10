#include <iostream>
#include <boost/thread.hpp>

// Create a thread by a function with no arguments.

void Hello() {
  // Sleep 1 second to simulate the data processing.
  boost::this_thread::sleep(boost::posix_time::seconds(1));
  std::cout << "Hello, World!" << std::endl;
}

int main() {
  // Create a thread object, and Hello will run immediately.
  boost::thread hello_thread(Hello);

  // Wait for the thread to finish.
  hello_thread.join();

  return 0;
}