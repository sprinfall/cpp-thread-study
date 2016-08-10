#include <iostream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

// Adapted from: libs/thread/tutorial/counter.cpp
// Lock and unlock a mutex directly.

boost::mutex mutex;
int count = 0;

void Counter() {
  mutex.lock();

  int i = ++count;
  std::cout << "count == " << i << std::endl;

  // If any exception was thrown, unlock won't be called.
  mutex.unlock();
}

int main() {
  // Create a group of counter threads.
  boost::thread_group threads;
  for (int i = 0; i < 4; ++i) {
    threads.create_thread(&Counter);
  }

  // Wait for all the threads to finish.
  threads.join_all();
  return 0;
}
