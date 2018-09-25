#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// Lock and unlock a mutex directly.

std::mutex g_mutex;
int g_count = 0;

void Counter() {
  g_mutex.lock();

  int i = ++g_count;
  std::cout << "count: " << i << std::endl;

  // If any exception was thrown, unlock won't be called.
  g_mutex.unlock();
}

int main() {
  const std::size_t SIZE = 4;

  // Create a group of counter threads.
  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Counter);
  }

  // Wait for all the threads to finish.
  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}
