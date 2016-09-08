#include <iostream>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// Implement semaphore based on mutex and condition variable.

// Adapted from:
// http://stackoverflow.com/questions/4792449/c0x-has-no-semaphores-how-to-synchronize-threads

// Boost: Why has class semaphore disappeared?
// http://www.boost.org/doc/libs/1_31_0/libs/thread/doc/faq.html

// https://en.wikipedia.org/wiki/Semaphore_%28programming%29#Semaphore_vs._mutex

class Semaphore {
public:
  Semaphore(long count = 0)
    : count_(count) {
  }

  void Signal() {
    boost::unique_lock<boost::mutex> lock(mutex_);
    ++count_;
    cv_.notify_one();
  }

  void Wait() {
    boost::unique_lock<boost::mutex> lock(mutex_);
    cv_.wait(lock, [=] { return count_ > 0; });
    --count_;
  }

private:
  boost::mutex mutex_;
  boost::condition_variable cv_;
  long count_;
};

Semaphore g_semaphore(3);
boost::mutex g_io_mutex;

std::string FormatTime(boost::posix_time::ptime& time, const char* format) {
  std::stringstream stream;
  boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
  facet->format(format);
  stream.imbue(std::locale(std::locale::classic(), facet));
  stream << time;
  return stream.str();
}

void Worker() {
  g_semaphore.Wait();

  boost::thread::id thread_id = boost::this_thread::get_id();

  std::string now = FormatTime(boost::posix_time::second_clock::local_time(), "%H:%M:%S");
  {
    boost::lock_guard<boost::mutex> lock(g_io_mutex);
    std::cout << "Thread " << thread_id << ": wait succeeded" << " (" << now << ")" << std::endl;
  }

  // Sleep 1 second to simulate data processing.
  boost::this_thread::sleep(boost::posix_time::seconds(1));

  g_semaphore.Signal();
}

int main() {
  boost::thread_group threads;
  for (int i = 0; i < 3; ++i) {
    threads.create_thread(&Worker);
  }

  threads.join_all();
  return 0;
}

//Thread 1d38: wait succeeded (13:10:10)
//Thread 20f4: wait succeeded (13:10:11)
//Thread 2348: wait succeeded (13:10:12)

//Thread 19f8: wait succeeded (13:10:57)
//Thread 2030: wait succeeded (13:10:57)
//Thread 199c: wait succeeded (13:10:57)