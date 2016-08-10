#include <iostream>
#include <boost/thread.hpp>

// Create a thread by a function object (functor).

class Hello {
public:
  void operator()(const char* what) {
    boost::this_thread::sleep(boost::posix_time::seconds(1));
    std::cout << "Hello, " << what << "!" << std::endl;
  }
};

int main() {
  Hello hello;

  // Copy the thread object.
  boost::thread hello_thread(hello, "World");
  hello_thread.join();

  // Don't copy the thread object, use boost::ref to pass in a reference.
  // But the user must ensure that the referred-to object outlives the
  // newly-created thread of execution. 
  boost::thread hello_thread_ref(boost::ref(hello), "World");
  hello_thread_ref.join();

  return 0;
}