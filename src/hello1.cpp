#include <iostream>
#include <thread>

// Create a thread by a function with no arguments.

void Hello() {
  // Sleep 1 second to simulate the data processing.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Hello, World!" << std::endl;
}

int main() {
  // Create a thread object, and Hello will run immediately.
  std::thread t(&Hello);

  // Wait for the thread to finish.
  t.join();

  return 0;
}