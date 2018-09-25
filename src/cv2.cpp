#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

// Adapted from: http://en.cppreference.com/w/cpp/thread/condition_variable

std::mutex mutex;
std::condition_variable cv;
std::string data;
bool ready = false;  // Shared variable
bool processed = false;  // Shared variable

void Worker() {
  std::unique_lock<std::mutex> lock(mutex);

  // Wait until main thread sends data.
  cv.wait(lock, [] { return ready; });
  // Equivalent to:
  //   while (!ready) { cv.wait(lock); }

  // After wait, we own the lock.
  std::cout << "Worker thread is processing data..." << std::endl;
  // Sleep 1 second to simulate data processing.
  std::this_thread::sleep_for(std::chrono::seconds(1));
  data += " processed";

  // Send data back to the main thread.
  processed = true;
  std::cout << "Worker thread signals data processing completed." << std::endl;

  // Manually unlock before notify to avoid waking up the waiting thread
  // only to block again (see notify_one for details).
  lock.unlock();

  cv.notify_one();
}

int main() {
  std::thread worker(Worker);

  // Send data to the worker thread.
  {
    std::lock_guard<std::mutex> lock(mutex);
    std::cout << "Main thread is preparing data..." << std::endl;
    // Sleep 1 second to simulate data preparation.
    std::this_thread::sleep_for(std::chrono::seconds(1));
    data = "Example data";
    ready = true;
    std::cout << "Main thread signals data ready for processing." << std::endl;
  }
  cv.notify_one();

  // Wait for the worker thread to process data.
  {
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [] { return processed; });
  }
  std::cout << "Back in main thread, data = " << data << std::endl;

  worker.join();

  return 0;
}
