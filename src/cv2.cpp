#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>

// Adapted from: http://en.cppreference.com/w/cpp/thread/condition_variable

boost::mutex mutex;
boost::condition_variable cv;
std::string data;
bool ready = false;  // Shared variable
bool processed = false;  // Shared variable

void Worker() {
  boost::unique_lock<boost::mutex> lock(mutex);

  // Wait until main thread sends data.
  cv.wait(lock, [] { return ready; });
  //while (!ready) {
  //  cv.wait(lock);
  //}

  // After wait, we own the lock.
  std::cout << "Worker thread is processing data..." << std::endl;
  // Sleep 1 second to simulate data processing.
  boost::this_thread::sleep_for(boost::chrono::seconds(1));
  data += " after processing";

  // Send data back to the main thread.
  processed = true;
  std::cout << "Worker thread signals data processing completed." << std::endl;

  // Manually unlock before notify to avoid waking up the waiting thread
  // only to block again (see notify_one for details).
  lock.unlock();

  cv.notify_one();
}

int main() {
  boost::thread worker(Worker);

  // Send data to the worker thread.
  {
    boost::lock_guard<boost::mutex> lock(mutex);
    std::cout << "Main thread is preparing data..." << std::endl;
    // Sleep 1 second to simulate data preparation.
    boost::this_thread::sleep_for(boost::chrono::seconds(1));
    data = "Example data";
    ready = true;
    std::cout << "Main thread signals data ready for processing." << std::endl;
  }
  cv.notify_one();

  // Wait for the worker thread to process data.
  {
    boost::unique_lock<boost::mutex> lock(mutex);
    cv.wait(lock, [] { return processed; });
  }
  std::cout << "Back in main thread, data = " << data << std::endl;

  worker.join();

  return 0;
}
