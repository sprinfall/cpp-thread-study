#include <iostream>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

// Adapted from: libs/thread/tutorial/counter.cpp
// Use lock_guard to automatically lock and unlock a mutex.

boost::mutex mutex;
int count = 0;

void Counter() {
  // lock_guard acquires the lock in constructor and releases it in destructor.
  boost::lock_guard<boost::mutex> lock(mutex);

  int i = ++count;
  std::cout << "count == " << i << std::endl;
}

int main() {
  boost::thread_group threads;
  for (int i = 0; i < 4; ++i) {
    threads.create_thread(&Counter);
  }

  threads.join_all();
  return 0;
}
