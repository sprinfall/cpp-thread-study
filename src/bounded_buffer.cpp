#include <iostream>
#include <vector>
#include <boost/utility.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread_only.hpp>

// Adapted from: libs/thread/example/condition.cpp

// The bounded-buffer problem, also known as producer¨Cconsumer.

// See:
// https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem
// http://stackoverflow.com/questions/9578050/bounded-buffers-producer-consumer
// http://stackoverflow.com/questions/9517405/empty-element-in-array-based-bounded-buffer

// Consider one sender and one receiver, the buffer size is 2.
//            buffered_    begin_      end_
// Init           0          0          0
// Send           1          0          1
// Receive        0          1          1
// Receive       Wait for buffered_ > 0 ...
// Send           1          1          0
// ...

class BoundedBuffer : private boost::noncopyable {
public:
  BoundedBuffer(size_t size)
    : begin_(0), end_(0), buffered_(0), circular_buffer_(size) {
  }

  void Send(int n) {
    boost::unique_lock<boost::mutex> lock(mutex_);
    not_full_cv_.wait(lock, [=] { return buffered_ < circular_buffer_.size(); });

    circular_buffer_[end_] = n;
    end_ = (end_ + 1) % circular_buffer_.size();

    ++buffered_;

    not_empty_cv_.notify_one();
  }

  int Receive() {
    boost::unique_lock<boost::mutex> lock(mutex_);
    not_empty_cv_.wait(lock, [=] { return buffered_ > 0; });

    int n = circular_buffer_[begin_];
    begin_ = (begin_ + 1) % circular_buffer_.size();

    --buffered_;

    not_full_cv_.notify_one();
    return n;
  }

private:
  size_t begin_;  // Receive index
  size_t end_;  // Send index
  size_t buffered_;  // Used buffer size
  std::vector<int> circular_buffer_;
  boost::condition_variable_any not_full_cv_;
  boost::condition_variable_any not_empty_cv_;
  boost::mutex mutex_;
};

BoundedBuffer buf(2);
boost::mutex io_mutex;

void Sender() {
  int n = 0;
  while (n < 100000) {
    buf.Send(n);
    if ((n % 10000) == 0) {
      boost::unique_lock<boost::mutex> lock(io_mutex);
      std::cout << "sent: " << n << std::endl;
    }
    ++n;
  }

  buf.Send(-1);
}

void Receiver() {
  boost::thread::id thread_id = boost::this_thread::get_id();

  int n;
  do {
    n = buf.Receive();
    if ((n % 10000) == 0) {
      boost::unique_lock<boost::mutex> lock(io_mutex);
      std::cout << "received: " << n << " (" << thread_id << ")" << std::endl;
    }
  } while (n != -1); // -1 indicates end of buffer.

  buf.Send(-1);  // For stopping next receiver.
}

int main() {
  boost::thread sender_thread(&Sender);
  boost::thread receiver_thread1(&Receiver);
  boost::thread receiver_thread2(&Receiver);
  boost::thread receiver_thread3(&Receiver);

  sender_thread.join();
  receiver_thread1.join();
  receiver_thread2.join();
  receiver_thread3.join();

  return 0;
}
