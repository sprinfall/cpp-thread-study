#include <iostream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/lock_guard.hpp>

// Adapted from: libs/thread/example/mutex.cpp
// Use a individual mutex for output stream.

boost::mutex mutex;
boost::mutex io_mutex;
int count = 0;

void Counter() {
  int i;
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    i = ++count;
  }

  {
    // The IO streams are not guaranteed to be thread-safe!
    // If you don't synchronize the IO, the output might look like:
    // count == count == 2count == 41
    // count == 3
    // Which means the output of "count == " and i is not atomic.
    boost::unique_lock<boost::mutex> lock(io_mutex);
    std::cout << "count == " << i << std::endl;
  }
}

int main() {
  boost::thread_group threads;
  for (int i = 0; i < 4; ++i) {
    threads.create_thread(&Counter);
  }

  threads.join_all();
  return 0;
}
