#include <condition_variable>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

// Wait a condition variable with timeout.

std::mutex mutex;
std::condition_variable cv;
// A flag used to stop worker thread from main thread.
bool stop = false;

void Worker() {
  std::unique_lock<std::mutex> lock(mutex);

  cv.wait_for(lock, std::chrono::seconds(3), [] { return stop; });

  // This is much better than:
  //   std::this_thread::sleep_for(std::chrono::seconds(3));
  //   if (stop) ...
  // or:
  //   for (std::size_t i = 0; i < 3 && !stop; ++i) {
  //     std::this_thread::sleep_for(std::chrono::seconds(1));
  //   }

  std::cout << "Worker thread is done (stop=" << (int)stop << ")" << std::endl;
}

int main() {
  std::thread worker(Worker);

  std::this_thread::sleep_for(std::chrono::seconds(1));

  stop = true;
  cv.notify_one();

  // The worker thread will be stopped immediately.
  worker.join();

  return 0;
}
