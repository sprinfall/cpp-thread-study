#include <functional>
#include <iostream>
#include <thread>

// Create a thread by a member function.

class Hello {
public:
  Hello() {
    std::thread t(std::bind(&Hello::Entry, this, "World"));
    t.join();
  }

private:
  void Entry(const char* what) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello, " << what << "!" << std::endl;
  }
};

int main() {
  Hello hello;

  return 0;
}