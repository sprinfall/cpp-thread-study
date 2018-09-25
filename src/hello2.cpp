#include <iostream>
#include <thread>

// Create a thread by a function with argument(s).

void Hello(const char* what) {
  // Sleep 1 second to simulate the data processing.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "Hello, " << what << "!" << std::endl;
}

int main() {
  std::thread t(&Hello, "World");

  // Equivalent to:
  //   std::thread t(std::bind(&Hello, "World"));

  t.join();

  return 0;
}