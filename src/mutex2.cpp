#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// Use lock_guard to automatically lock and unlock a mutex.

std::mutex g_mutex;
int g_count = 0;

void Counter() {
  // lock_guard acquires the lock in constructor and releases it in destructor.
  std::lock_guard<std::mutex> lock(g_mutex);

  int i = ++g_count;
  std::cout << "count: " << i << std::endl;
}

int main() {
  const std::size_t SIZE = 4;

  std::vector<std::thread> v;
  v.reserve(SIZE);

  for (std::size_t i = 0; i < SIZE; ++i) {
    v.emplace_back(&Counter);
  }

  for (std::thread& t : v) {
    t.join();
  }

  return 0;
}
