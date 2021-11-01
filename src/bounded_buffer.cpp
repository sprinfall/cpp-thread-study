#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

// The bounded-buffer problem, also known as producer¨Cconsumer.

// See:
// https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
// http://stackoverflow.com/questions/9578050/bounded-buffers-producer-consumer
// http://stackoverflow.com/questions/9517405/empty-element-in-array-based-bounded-buffer

// Consider one producer and one consumer, the buffer size is 2.
//            buffered_    begin_      end_
// Init           0          0          0
// Produce        1          0          1
// Consume        0          1          1
// Consume       Wait for buffered_ > 0 ...
// Produce        1          1          0
// ...

class BoundedBuffer {
public:
  BoundedBuffer(const BoundedBuffer& rhs) = delete;
  BoundedBuffer& operator=(const BoundedBuffer& rhs) = delete;

  BoundedBuffer(std::size_t size)
      : begin_(0), end_(0), buffered_(0), circular_buffer_(size) {
  }

  void Produce(int n) {
    {
      std::unique_lock<std::mutex> lock(mutex_);
      not_full_cv_.wait(lock, [this] { return buffered_ < circular_buffer_.size(); });

      circular_buffer_[end_] = n;
      end_ = (end_ + 1) % circular_buffer_.size();

      ++buffered_;
    }

    not_empty_cv_.notify_one();
  }

  int Consume() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_cv_.wait(lock, [this] { return buffered_ > 0; });

    int n = circular_buffer_[begin_];
    begin_ = (begin_ + 1) % circular_buffer_.size();

    --buffered_;

    lock.unlock();
    not_full_cv_.notify_one();
    return n;
  }

private:
  std::size_t begin_;
  std::size_t end_;
  std::size_t buffered_;
  std::vector<int> circular_buffer_;
  std::condition_variable not_full_cv_;
  std::condition_variable not_empty_cv_;
  std::mutex mutex_;
};

BoundedBuffer g_buffer(2);
std::mutex g_io_mutex;

void Producer() {
  int n = 0;
  while (n < 100000) {
    g_buffer.Produce(n);
    if ((n % 10000) == 0) {
      std::unique_lock<std::mutex> lock(g_io_mutex);
      std::cout << "Produce: " << n << std::endl;
    }
    ++n;
  }

  g_buffer.Produce(-1);
}

void Consumer() {
  std::thread::id thread_id = std::this_thread::get_id();

  int n = 0;
  do {
    n = g_buffer.Consume();
    if ((n % 10000) == 0) {
      std::unique_lock<std::mutex> lock(g_io_mutex);
      std::cout << "Consume: " << n << " (" << thread_id << ")" << std::endl;
    }
  } while (n != -1);  // -1 indicates end of buffer.

  g_buffer.Produce(-1);  // For stopping next consumer.
}

int main() {
  std::vector<std::thread> threads;

  threads.push_back(std::thread(&Producer));
  threads.push_back(std::thread(&Consumer));
  threads.push_back(std::thread(&Consumer));
  threads.push_back(std::thread(&Consumer));

  for (auto& t : threads) {
    t.join();
  }

  return 0;
}

// Output:
// Produce: 0
// Consume: 0 (7244)
// Produce: 10000
// Consume: 10000 (13988)
// Produce: 20000
// Consume: 20000 (13988)
// Produce: 30000
// Consume: 30000 (7244)
// Produce: 40000
// Consume: 40000 (13988)
// Produce: 50000
// Consume: 50000 (16660)
// Produce: 60000
// Consume: 60000 (7244)
// Produce: 70000
// Consume: 70000 (16660)
// Produce: 80000
// Consume: 80000 (13988)
// Produce: 90000
// Consume: 90000 (13988)
