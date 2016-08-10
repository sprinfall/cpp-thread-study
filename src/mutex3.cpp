#include <iostream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

// Adapted from: libs/thread/tutorial/counter.cpp

// Use unique_lock to automatically lock and unlock a mutex.
// unique_lock provides more features than lock_guard.
// NOTE: mutex::scoped_lock is just a typedef of unique_lock<mutex>.

// unique_lock vs. lock_guard:
// http://stackoverflow.com/questions/6731027/boostunique-lock-vs-boostlock-guard

boost::mutex mutex;
int count = 0;

void Counter() {
  boost::unique_lock<boost::mutex> lock(mutex);

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
